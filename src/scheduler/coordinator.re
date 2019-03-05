type t = {
  should_halt: bool,
  policy: Policy.t,
  workers: Worker_registry.t,
  tasks: Task_queue.t(Bytecode.t),
};

let __global_coordinator: ref(t) =
  ref({
    should_halt: false,
    policy: Policy.default(),
    workers: Worker_registry.create(),
    tasks: Task_queue.create(),
  });

let current = () => __global_coordinator^;

let prepare = () => {
  let {policy, workers, _} = current();

  Policy.worker_count(policy)
  |> (size => Array.make(size, None))
  |> Array.iteri((_, _) =>
       switch (Worker.Child.current()) {
       | Some(_) => ()
       | None =>
         switch (Worker.start()) {
         | Some(worker) =>
           let worker_id = worker |> Worker.id |> Int32.of_int;
           Worker_registry.register(workers, worker_id, worker) |> ignore;
         | None => ()
         }
       }
     );

  switch (Worker.Child.current()) {
  | Some(_) => ()
  | None =>
    Logs.info(m =>
      m(
        "[%d] Spawned %d out of %d workers: %s",
        Platform.Process.pid(),
        policy |> Policy.worker_count,
        workers |> Worker_registry.size,
        workers
        |> Worker_registry.ids
        |> Seq.map(Int32.to_string)
        |> List.of_seq
        |> String.concat(", "),
      )
    )
  };
};

let kill_workers: Seq.t(Worker.t) => unit =
  wrkrs => {
    wrkrs |> Seq.map(Worker.id) |> Seq.iter(Platform.Process.kill);
  };

let halt_system: unit => unit =
  () => {
    __global_coordinator := {...__global_coordinator^, should_halt: true};
  };

let setup = policy => {
  __global_coordinator := {...__global_coordinator^, policy};
  prepare();
};

let enter_loop = () => {
  let {workers, tasks, _} = current();
  Logs.debug(m =>
    m("[%d] Beginning scheduling loop...", Platform.Process.pid())
  );

  let rec do_loop = () => {
    let {should_halt, _} = current();
    switch (should_halt) {
    | true => ()
    | _ =>
      let next =
        Worker_registry.workers(workers) |> Worker.wait_next_available;
      switch (next) {
      | `Receive(cmds) =>
        cmds |> Seq.fold_left(Task_queue.queue, tasks) |> ignore
      | `Send(wrkrs) =>
        switch (Task_queue.next(tasks)) {
        | None => ()
        | Some(Bytecode.Halt as task) =>
          halt_system();
          wrkrs |> Seq.iter(Worker.send_task(task));
        | Some(task) => wrkrs |> Seq.iter(Worker.send_task(task))
        }
      | `Wait => ()
      };
      do_loop();
    };
  };

  do_loop();
};

module Tasks = {
  let send_message = (~pid, ~msg) => {
    switch (Worker.Child.current()) {
    | Some(worker) =>
      `From_worker(Bytecode.Send_message(pid, msg))
      |> Task_queue.queue(worker.tasks)
      |> ignore
    | None =>
      let coordinator = __global_coordinator^;
      Logs.debug(m =>
        m(
          "[%d] Sending message to pid %s",
          Platform.Process.pid(),
          pid |> Model.Pid.to_string,
        )
      );
      Bytecode.Send_message(pid, msg)
      |> Task_queue.queue(coordinator.tasks)
      |> ignore;
    };
  };

  let spawn_from_coordinator = (task, state) => {
    let coordinator = current();
    let least_busy_worker =
      coordinator.workers
      |> Worker_registry.workers
      |> List.of_seq
      |> Worker.least_busy;

    let pid =
      switch (least_busy_worker) {
      | Some(worker) =>
        let (worker', new_pid) = worker |> Worker.next_pid;
        let worker_id = worker' |> Worker.id |> Int32.of_int;

        Worker_registry.update(coordinator.workers, worker_id, worker')
        |> ignore;

        Bytecode.Spawn(new_pid, task, state)
        |> Task_queue.queue(coordinator.tasks)
        |> ignore;

        new_pid;
      | None => Model.Pid.make(0, 0, 0)
      };

    pid;
  };

  let spawn_in_worker:
    (Worker.Child.t, Model.Process.task('a), 'a) => Model.Pid.t =
    (worker, task, state) => {
      let new_pid = worker.last_pid^ |> Model.Pid.next;
      worker.last_pid := new_pid;

      `From_worker(Bytecode.Spawn(new_pid, task, state))
      |> Task_queue.queue(worker.tasks)
      |> ignore;
      new_pid;
    };

  let spawn = (~task, ~state) => {
    let pid =
      switch (Worker.Child.current()) {
      | Some(worker) => spawn_in_worker(worker, task, state)
      | None => spawn_from_coordinator(task, state)
      };

    Logs.debug(m =>
      m(
        "[%d] Spawning process with pid %s",
        Platform.Process.pid(),
        pid |> Model.Pid.to_string,
      )
    );

    pid;
  };

  let halt = () => {
    Logs.debug(m => m("[%d] Shutting down...", Platform.Process.pid()));

    switch (Worker.Child.current()) {
    | Some(worker) =>
      let tasks' = Task_queue.clear(worker.tasks);
      `From_worker(Bytecode.Halt) |> Task_queue.queue(tasks') |> ignore;
    | None =>
      let coordinator = current();
      let tasks' = Task_queue.clear(coordinator.tasks);
      Bytecode.Halt |> Task_queue.queue(tasks') |> ignore;
    };
  };

  let run = () => {
    switch (Worker.Child.current()) {
    | Some(worker) =>
      Logs.info(m => m("[%d] Worker shutting down...", worker.unix_pid));
      exit(0);
    | None =>
      enter_loop();
      kill_workers(current().workers |> Worker_registry.workers);
      Logs.info(m =>
        m("[%d] Scheduler shutting down...", Platform.Process.pid())
      );
    };
  };
};
