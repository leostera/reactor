/**
  The type of the global coordinator
  */
type t;

let configure: Policy.t => unit;

let start: unit => unit;

let current: unit => t;

let worker_for_pid: (t, ~pid: Model.Pid.t) => option(Worker.t);

let handle_task: (t, ~task: Bytecode.t) => unit;
