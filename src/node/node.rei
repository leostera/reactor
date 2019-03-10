/**
  A node is a {i coordinator} of schedulers, with an identity of its own.
  */
type t;

/**
  [role()] will check the current global state and determine if the current
  process is a [`Node] or a [`Scheduler].
  */
let role: unit => [ | `Node(t) | `Scheduler(Reactor_scheduler.Scheduler.t)];

let run: unit => unit;

let setup: Policy.t => unit;

let enqueue: (t, Reactor_bytecode.Bytecode.t) => unit;

let halt: t => unit;

let least_busy_scheduler: t => option(Scheduler_view.t);

let next_pid: (t, Scheduler_view.t) => Reactor_process.Process.Pid.t;

module Policy: (module type of Policy);
