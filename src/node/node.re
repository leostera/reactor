module Policy = Policy;

type t = {
  unix_pid: int,
  should_halt: bool,
  policy: Policy.t,
  schedulers: Scheduler_view.Registry.t,
  tasks: Task_queue.t(Bytecode.t),
};

let __is_node: ref(bool) = ref(true);

let __node: ref(t) =
  ref({
    unix_pid: Platform.Process.pid(),
    policy: Policy.default(),
    should_halt: false,
    schedulers: Scheduler_view.Registry.create(),
    tasks: Task_queue.create(),
  });

let current = () => __node^;

let set_policy = policy => {
  __node := {...current(), policy};
};

let role = () =>
  switch (Scheduler.current()) {
  | Some(s) => `Scheduler(s)
  | None => `Node(current())
  };

let enqueue = (t, task) => Task_queue.queue(t.tasks, task);

let halt = t => {
  Task_queue.clear(t.tasks);
  enqueue(t, Bytecode.Halt);
};

let next_pid = (t, scheduler) => {
  let (scheduler', new_pid) = scheduler |> Scheduler_view.next_pid;
  let scheduler_id = scheduler' |> Scheduler_view.id |> Int32.of_int;
  Scheduler_view.Registry.update(t.schedulers, scheduler_id, scheduler');
  new_pid;
};

let least_busy_scheduler = t => t.schedulers |> Scheduler_view.least_busy;

let setup = policy => {
  set_policy(policy);
  let {policy, schedulers, _} = current();

  let mark_as_scheduler = () => {
    __is_node := false;
  };

  let on_child = (pid, to_parent, from_parent) => {
    mark_as_scheduler();
    Scheduler.setup(~pid, to_parent, from_parent)
    |> Scheduler.run(to_parent, from_parent);
  };

  let on_parent = Scheduler_view.make;

  let after_child_spawn = scheduler => {
    let scheduler_id = scheduler |> Scheduler_view.id |> Int32.of_int;
    Scheduler_view.Registry.register(schedulers, scheduler_id, scheduler);
  };

  Coordinator.create_pool(
    ~child_count=Policy.scheduler_count(policy),
    ~is_child=() => __is_node^ != true,
    ~on_child,
    ~on_parent,
    ~after_child_spawn,
  );

  switch (role()) {
  | `Scheduler(_) => ()
  | `Node(_) =>
    Logs.info(m => {
      let pid = Platform.Process.pid();
      let count = policy |> Policy.scheduler_count;
      let current_count = schedulers |> Scheduler_view.Registry.size;
      let ids =
        schedulers
        |> Scheduler_view.Registry.keys
        |> Seq.map(Int32.to_string)
        |> List.of_seq
        |> String.concat(", ");
      m("[%d] Spawned %d/%d schedulers: %s", pid, current_count, count, ids);
    })
  };
};

let kill_schedulers = wrkrs => {
  let pids = wrkrs |> Seq.map(Scheduler_view.unix_pid) |> List.of_seq;
  pids |> List.iter(Platform.Process.kill);
  Logs.info(m =>
    m(
      "[%d] Waiting for %d schedulers to finish...",
      Platform.Process.pid(),
      pids |> List.length,
    )
  );
  Platform.Process.wait(pids);
};

let mark_system_to_halt_on_next_tick = () => {
  __node := {...__node^, should_halt: true};
};

let enter_loop = node => {
  let {schedulers, unix_pid, _} = node;
  Logs.info(m => m("[%d] Beginning node's coordinating loop...", unix_pid));

  let (reads, writes) =
    schedulers
    |> Scheduler_view.Registry.values
    |> Seq.map(Scheduler_view.pipes)
    |> Seq.fold_left(Scheduler_view.fold_pipes, ([], []));

  let rec do_loop = () => {
    let {tasks, should_halt, _} = current();
    switch (should_halt) {
    | true => ()
    | _ =>
      let (reads, writes) = Coordinator.wait_next_available(reads, writes);

      switch (reads) {
      | `Receive(cmds) => List.iter(Task_queue.queue(tasks), cmds)
      | `Wait => ()
      };

      switch (writes) {
      | `Send(wrkr_fds) =>
        let send_task = t => List.iter(Coordinator.send_task(t), wrkr_fds);
        switch (Task_queue.next(tasks)) {
        | None => ()
        | Some(Bytecode.Halt as task) =>
          mark_system_to_halt_on_next_tick();
          send_task(task);
        | Some(task) => send_task(task)
        };
      | `Wait => ()
      };

      do_loop();
    };
  };

  do_loop();
};

let shutdown = node => {
  Logs.info(m => m("[%d] Node shutting down...", node.unix_pid));
  __node :=
    {
      ...node,
      should_halt: false,
      schedulers: Scheduler_view.Registry.create(),
      tasks: Task_queue.create(),
    };
  kill_schedulers(node.schedulers |> Scheduler_view.Registry.values);
  Logs.info(m => m("[%d] Node shutdown completed.", node.unix_pid));
};

let run = () => {
  switch (role()) {
  | `Scheduler(scheduler) =>
    let id = scheduler |> Scheduler.id;
    Logs.info(m => m("[%d] Scheduler shutting down...", id));
    exit(0);
  | `Node(node) =>
    enter_loop(node);
    shutdown(node);
  };
};
