module Node = Reactor_node.Node;
module Task_queue = Reactor_task_queue.Task_queue;
module Scheduler = Reactor_scheduler.Scheduler;
module Bytecode = Reactor_bytecode.Bytecode;
module Process = Reactor_process.Process;

let send = (pid, msg) => {
  let msg = Bytecode.Send_message(pid, msg);
  switch (Node.role()) {
  | `Scheduler(s) => Scheduler.enqueue(s, `From_scheduler(msg))
  | `Node(n) => Node.enqueue(n, msg)
  };
};

let (<-) = send;

let spawn_from_coordinator = (node, task, state) => {
  let least_busy_scheduler = node |> Node.least_busy_scheduler;

  let pid =
    switch (least_busy_scheduler) {
    | Some(scheduler) =>
      let pid = Node.next_pid(node, scheduler);
      Node.enqueue(node, Bytecode.Spawn(pid, task, state));
      pid;
    | None => Process.Pid.make(0, 0, 0)
    };

  pid;
};

let spawn_in_scheduler = (scheduler, task, state) => {
  let pid = Scheduler.next_pid(scheduler);
  let msg = Bytecode.Spawn(pid, task, state);
  Scheduler.enqueue(scheduler, `From_scheduler(msg));
  pid;
};

let spawn = (task, state) => {
  let pid =
    switch (Node.role()) {
    | `Scheduler(scheduler) => spawn_in_scheduler(scheduler, task, state)
    | `Node(node) => spawn_from_coordinator(node, task, state)
    };
  pid;
};

let halt = () => {
  Logs.debug(m => m("[%d] Shutting down...", Platform.Process.pid()));
  switch (Node.role()) {
  | `Scheduler(scheduler) => Scheduler.halt(scheduler)
  | `Node(node) => Node.halt(node)
  };
};

let exit = () => {
  halt();
  `Terminate;
};
