module Unix = UnixLabels;

type t = {
  policy: Policy.t,
  workers: Worker_registry.t,
  tasks: Task_queue.t(Bytecode.t),
};

let __global_coordinator: ref(t) =
  ref({
    policy: Policy.default(),
    workers: Worker_registry.create(),
    tasks: Task_queue.create(),
  });

let current = () => __global_coordinator^;

let configure = policy => {
  __global_coordinator := {...__global_coordinator^, policy};
};

let loop = (workers, tasks) => {
  Logs.debug(m => m("Beginning Scheduling loop..."));

  let rec do_loop = () => {
    let next = Worker_registry.workers(workers) |> Worker.wait_next_available;
    switch (next) {
    | `Receive(cmds) =>
      cmds
      |> Seq.fold_left((q, cmd) => Task_queue.queue(q, cmd), tasks)
      |> ignore;

      let task_list =
        Task_queue.to_seq(tasks)
        |> Seq.map(Bytecode.to_string)
        |> List.of_seq;

      Logs.debug(m =>
        m("Current Task Queue (%d tasks):", task_list |> List.length)
      );

      Logs.debug(m => m("%s", task_list |> String.concat("\n\t —")));

      do_loop();

    | _ =>
      /* _wrks |> Seq.iter(Worker.send_task(task)); */

      do_loop()
    };
  };

  do_loop();
};

let start = () => {
  let {policy, workers, tasks} = current();

  Policy.worker_count(policy)
  |> (size => Array.make(size, None))
  |> Array.iteri((_, _) =>
       switch (Worker.start()) {
       | Some(worker) =>
         let worker_id = worker |> Worker.id |> Int32.of_int;
         Worker_registry.register(workers, worker_id, worker) |> ignore;
       | None => ()
       }
     );

  Logs.app(m =>
    m(
      "Spawned %d out of %d workers: %s",
      policy |> Policy.worker_count,
      workers |> Worker_registry.size,
      workers
      |> Worker_registry.ids
      |> Seq.map(Int32.to_string)
      |> List.of_seq
      |> String.concat(", "),
    )
  );

  loop(workers, tasks);
};

let worker_for_pid = (coordinator, ~pid) => {
  let {worker_id, _}: Model.Pid.view = Model.Pid.view(pid);
  Worker_registry.find(coordinator.workers, worker_id);
};

let handle_task = (coordinator, ~task) => {
  Task_queue.queue(coordinator.tasks, task) |> ignore;
};
