module Task_queue = Reactor_task_queue.Task_queue;
module Coordinator = Reactor_coordinator.Coordinator;

type task = [
  | `From_coordinator(Bytecode.t)
  | `From_scheduler(Bytecode.t)
  | `Reduction(unit => unit)
];

type t = {
  id: int,
  unix_pid: int,
  pipe_to_coordinator: Unix.file_descr,
  pipe_from_coordinator: Unix.file_descr,
  should_halt: ref(bool),
  last_pid: ref(Process.Pid.t),
  processes: Process.Registry.t,
  process_count: int,
  tasks: Task_queue.t(task),
};

let __scheduler: ref(option(t)) = ref(None);

let current = () => __scheduler^;

let is_scheduler = () =>
  switch (current()) {
  | None => false
  | _ => true
  };

let id = t => t.id;

let next_pid = t => {
  let pid = Process.Pid.next(t.last_pid^);
  t.last_pid := pid;
  pid;
};

let last_pid = t => t.last_pid^;

let enqueue = (t, task) => Task_queue.queue(t.tasks, task);

let halt = t => {
  Task_queue.clear(t.tasks);
  enqueue(t, `From_scheduler(Bytecode.Halt));
};

let setup = (~pid, `Write(to_parent), `Read(from_parent)) => {
  let scheduler = {
    id: pid,
    unix_pid: pid,
    pipe_to_coordinator: to_parent,
    pipe_from_coordinator: from_parent,
    should_halt: ref(false),
    last_pid: ref(Process.Pid.make(0, pid, 0)),
    processes: Process.Registry.create(),
    process_count: 0,
    tasks: Task_queue.create(),
  };
  __scheduler := Some(scheduler);
  scheduler;
};

let handle_spawn = (scheduler, pid, task, state) => {
  let proc = Process.make(pid);
  let env = Process.{self: () => pid, recv: Process.recv(proc)};

  Process.Registry.register(scheduler.processes, pid, proc);

  let queue_reduction = f => {
    let reduction = `Reduction(f);
    Task_queue.queue(scheduler.tasks, reduction);
  };

  let terminate = () => Process.Registry.unregister(scheduler.processes, pid);

  let rec run_process = args => {
    switch (task(env, args)) {
    | exception ex =>
      Logs.debug(m =>
        m(
          "Process Terminated: %s threw %s",
          pid |> Process.Pid.to_string,
          ex |> Printexc.to_string,
        )
      )
    | `Terminate => terminate()
    | `Become(next_state) => queue_reduction(() => run_process(next_state))
    | `Defer(next_state_promise) =>
      queue_reduction(() => {
        Lwt.on_success(next_state_promise, next_state =>
          queue_reduction(() => run_process(next_state))
        );
        Lwt.on_failure(next_state_promise, _ => terminate());
      })
    };
  };
  run_process(state);
};

let handle_send_message = (scheduler, pid, msg) => {
  switch (Process.Registry.find(scheduler.processes, pid)) {
  | None => ()
  | Some(proc) => Process.send(proc, msg)
  };
};

let handle_halt = scheduler => {
  scheduler.should_halt := true;
};

let should_handle_task_locally = (scheduler, task) => {
  switch (task) {
  | Bytecode.Halt => false
  | Bytecode.Send_message(pid, _)
  | Bytecode.Spawn(pid, _, _) =>
    let {Process.Pid.scheduler_id, _} = pid |> Process.Pid.view;
    scheduler.unix_pid |> Int32.of_int |> Int32.equal(scheduler_id);
  };
};

let handle_coordinator_task = (scheduler, `From_coordinator(task)) => {
  switch (should_handle_task_locally(scheduler, task), task) {
  | (true, Bytecode.Send_message(pid, msg)) =>
    handle_send_message(scheduler, pid, msg)
  | (true, Bytecode.Spawn(pid, proc, state)) =>
    handle_spawn(scheduler, pid, proc, state)
  | (false, Bytecode.Halt) => handle_halt(scheduler)
  | (_, _) => ()
  };
};

let handle_scheduler_task =
    (scheduler, `Write(to_parent), `From_scheduler(task)) => {
  switch (should_handle_task_locally(scheduler, task), task) {
  | (true, Bytecode.Send_message(pid, msg)) =>
    handle_send_message(scheduler, pid, msg)
  | (true, Bytecode.Spawn(pid, proc, state)) =>
    handle_spawn(scheduler, pid, proc, state)
  | (false, Bytecode.Halt as task) =>
    handle_halt(scheduler);
    Coordinator.send_task(task, to_parent);
  | (_, task) => Coordinator.send_task(task, to_parent)
  };
};

let handle_task = (scheduler, `Write(_) as fd, task) => {
  switch (task) {
  | `From_coordinator(_) as task => handle_coordinator_task(scheduler, task)
  | `From_scheduler(_) as task => handle_scheduler_task(scheduler, fd, task)
  | `Reduction(f) => f()
  };
};

let run = (`Write(to_parent), `Read(from_parent), scheduler) => {
  Logs.info(m => m("[%d] Beginning scheduler loop...", scheduler.unix_pid));
  let rec do_loop = () => {
    switch (scheduler.should_halt^) {
    | true => ()
    | _ =>
      Logs.debug(m =>
        m(
          "[%i] Tasks queue has %d tasks",
          scheduler.unix_pid,
          scheduler.tasks |> Task_queue.length,
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
        Logs.debug(m => m("[%i] Receiving tasks...", scheduler.unix_pid));
        let cmd: Bytecode.t =
          Coordinator.Packet.read_from_pipe(`Read(from_parent));
        Task_queue.queue(scheduler.tasks, `From_coordinator(cmd));
      | _ => ()
      };

      switch (scheduler.tasks |> Task_queue.length, write_fds) {
      | (x, fds) when List.length(fds) > 0 && x > 0 =>
        switch (Task_queue.next(scheduler.tasks)) {
        | None =>
          Logs.debug(m =>
            m("[%i] No tasks to send. Standing by.", scheduler.unix_pid)
          )
        | Some(task) =>
          Logs.debug(m => m("[%i] Handling tasks...", scheduler.unix_pid));
          handle_task(scheduler, `Write(to_parent), task);
        }
      | _ => ()
      };

      Lwt_engine.iter(false);
      do_loop();
    };
  };
  switch (do_loop()) {
  | exception e =>
    Logs.err(m => {
      let err = Printexc.to_string(e);
      m("[%i] Uncaught exception in scheduler: %s", scheduler.unix_pid, err);
    });
    exit(1);
  | x => x
  };
};
