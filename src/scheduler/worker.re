module Unix = UnixLabels;

type t = {
  id: int,
  unix_pid: int,
  pipe_to_worker: Unix.file_descr,
  pipe_from_worker: Unix.file_descr,
  last_pid: Model.Pid.t,
  process_count: int,
};

let id = t => t.id;

let last_pid = t => t.last_pid;

let least_busy = workers => {
  let by_process_count = (w1, w2) => w1.process_count - w2.process_count;
  let sorted = workers |> List.sort(by_process_count);
  switch (sorted) {
  | [hd, ..._] => Some(hd)
  | _ => None
  };
};

let wait_next_available = workers => {
  let read_fds =
    workers |> Seq.fold_left((fds, w) => [w.pipe_from_worker, ...fds], []);

  let write_fds =
    workers |> Seq.fold_left((fds, w) => [w.pipe_to_worker, ...fds], []);

  switch (
    Platform.Process.select(
      ~read=read_fds,
      ~write=write_fds,
      ~except=[],
      ~timeout=-1.0,
    )
  ) {
  | (`Read(ins), _, _) when List.length(ins) > 0 =>
    let cmds: Seq.t(Bytecode.t) =
      ins |> List.map(fd => Packet.read_from_pipe(`Read(fd))) |> List.to_seq;

    `Receive(cmds);

  | (_, `Write(outs), _) when List.length(outs) > 0 =>
    let wrkrs = workers |> Seq.filter(w => List.mem(w.pipe_to_worker, outs));
    `Send(wrkrs);
  | _ => `Wait
  };
};

module Child = {
  type task = [
    | `From_coordinator(Bytecode.t)
    | `From_worker(Bytecode.t)
    | `Reduction(unit => unit)
  ];
  type t = {
    id: int,
    unix_pid: int,
    pipe_to_coordinator: Unix.file_descr,
    pipe_from_coordinator: Unix.file_descr,
    last_pid: ref(Model.Pid.t),
    processes: Model.Registry.t,
    process_count: int,
    tasks: Task_queue.t(task),
  };

  let __global_worker: ref(option(t)) = ref(None);

  let current = () => __global_worker^;

  let setup = (~pid, `Write(to_parent), `Read(from_parent)) => {
    __global_worker :=
      Some({
        id: pid,
        unix_pid: pid,
        pipe_to_coordinator: to_parent,
        pipe_from_coordinator: from_parent,
        last_pid: ref(Model.Pid.make(0, pid, 0)),
        processes: Model.Registry.create(),
        process_count: 0,
        tasks: Task_queue.create(),
      });
    __global_worker^;
  };

  let handle_spawn = (worker, pid, task, state) => {
    open Model;

    let proc = Process.make(pid);
    let env = Process.{self: () => pid, recv: Model.Process.recv(proc)};

    Registry.register(worker.processes, pid, proc) |> ignore;

    let rec run_process = args => {
      let queue_reduction = next_state =>
        Task_queue.queue(
          worker.tasks,
          `Reduction(
            () => {
              Logs.debug(m =>
                m("%s executing reduction", pid |> Pid.to_string)
              );

              run_process(next_state);
            },
          ),
        )
        |> ignore;

      let terminate = () =>
        Registry.unregister(worker.processes, pid) |> ignore;

      switch (task(env, args)) {
      | exception ex =>
        Logs.debug(m =>
          m(
            "Process Terminated: %s threw %s",
            pid |> Pid.to_string,
            ex |> Printexc.to_string,
          )
        )
      | `Terminate => terminate()
      | `Become(next_state) => queue_reduction(next_state)
      | `Defer(next_state_promise) =>
        Lwt.on_success(next_state_promise, queue_reduction);
        Lwt.on_failure(next_state_promise, _ => terminate());
        Logs.debug(m =>
          m("Scheduled defer callbacks in %s", pid |> Pid.to_string)
        );
      };
    };
    run_process(state);
  };

  let handle_send_message = (worker, pid, msg) => {
    switch (Model.Registry.find(worker.processes, pid)) {
    | None => ()
    | Some(proc) =>
      /** NOTE: send mutates the processes mailbox */
      Model.Process.send(proc, msg);
      Logs.debug(m => m("Sent message to %s", pid |> Model.Pid.to_string));
    };
  };

  let should_handle_task_locally = task => {
    switch (task) {
    | Bytecode.Send_message(pid, _)
    | Bytecode.Spawn(pid, _, _) =>
      let {Model.Pid.worker_id, _} = pid |> Model.Pid.view;
      Platform.Process.pid() |> Int32.of_int |> Int32.equal(worker_id);
    };
  };

  let handle_coordinator_task = (worker, `From_coordinator(task)) => {
    switch (should_handle_task_locally(task), task) {
    | (true, Bytecode.Send_message(pid, msg)) =>
      handle_send_message(worker, pid, msg)
    | (true, Bytecode.Spawn(pid, proc, state)) =>
      handle_spawn(worker, pid, proc, state)
    | (false, _) => ()
    };
  };

  let handle_worker_task = (worker, `Write(to_parent), `From_worker(task)) => {
    switch (should_handle_task_locally(task), task) {
    | (true, Bytecode.Send_message(pid, msg)) =>
      handle_send_message(worker, pid, msg)
    | (true, Bytecode.Spawn(pid, proc, state)) =>
      handle_spawn(worker, pid, proc, state)
    | (false, task) =>
      /** should send message back to coordinator */
      let cmd = Packet.encode(task);
      Platform.Process.write(to_parent, ~buf=cmd);
    };
  };

  let handle_task = (worker, `Write(_) as fd, task) => {
    switch (task) {
    | `From_coordinator(_) as task => handle_coordinator_task(worker, task)
    | `From_worker(_) as task => handle_worker_task(worker, fd, task)
    | `Reduction(f) => f()
    };
  };

  let run = (worker, `Write(to_parent), `Read(from_parent)) => {
    let pid = Platform.Process.pid();
    let rec do_loop = () => {
      Logs.debug(m =>
        m(
          "[%i] Tasks queue has %d tasks",
          worker.unix_pid,
          worker.tasks |> Task_queue.length,
        )
      );

      let (`Read(read_fds), `Write(write_fds), _) =
        Platform.Process.select(
          ~read=[from_parent],
          ~write=[to_parent],
          ~except=[],
          ~timeout=-1.0,
        );

      switch (read_fds) {
      | fds when List.length(fds) > 0 =>
        Logs.debug(m => m("[%i] Receiving tasks...", worker.unix_pid));
        let cmd: Bytecode.t = Packet.read_from_pipe(`Read(from_parent));
        Task_queue.queue(worker.tasks, `From_coordinator(cmd)) |> ignore;
      | _ => ()
      };

      switch (worker.tasks |> Task_queue.length, write_fds) {
      | (x, fds) when List.length(fds) > 0 && x > 0 =>
        switch (Task_queue.next(worker.tasks)) {
        | None =>
          Logs.debug(m =>
            m("[%i] No tasks to send. Standing by.", worker.unix_pid)
          )
        | Some(task) =>
          Logs.debug(m => m("[%i] Handling tasks...", worker.unix_pid));
          handle_task(worker, `Write(to_parent), task);
        }
      | _ => ()
      };

      Lwt_engine.iter(false);
      do_loop();
    };
    switch (do_loop()) {
    | exception e =>
      let err = Printexc.to_string(e);
      Logs.err(m => m("Uncaught exception in worker (pid %i): %s", pid, err));
      exit(1);
    | x => x
    };
  };
};

let start = () => {
  switch (Platform.Process.piped_fork()) {
  | `In_child(to_parent, from_parent) =>
    let worker =
      Child.setup(~pid=Platform.Process.pid(), to_parent, from_parent);
    switch (worker) {
    | Some(w) => Child.run(w, to_parent, from_parent) |> ignore
    | None => ()
    };
    None;
  | `In_parent(`Child_pid(pid), `Write(to_worker), `Read(from_worker)) =>
    Some({
      id: pid,
      unix_pid: pid,
      pipe_to_worker: to_worker,
      pipe_from_worker: from_worker,
      last_pid: Model.Pid.make(0, pid, 0),
      process_count: 0,
    })
  };
};

let send_task = (task, worker) => {
  let cmd = Packet.encode(task);
  Platform.Process.write(worker.pipe_to_worker, ~buf=cmd);
};

let next_pid = worker => {
  let pid = worker.last_pid |> Model.Pid.next;
  ({...worker, last_pid: pid, process_count: worker.process_count + 1}, pid);
};
