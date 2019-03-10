type t;

type task = [
  | `From_coordinator(Bytecode.t)
  | `From_scheduler(Bytecode.t)
  | `Reduction(unit => unit)
];

let id: t => int;

let last_pid: t => Process.Pid.t;

let next_pid: t => Process.Pid.t;

let current: unit => option(t);

let is_scheduler: unit => bool;

let enqueue: (t, task) => unit;

let halt: t => unit;

let setup:
  (~pid: int, [< | `Write(Unix.file_descr)], [< | `Read(Unix.file_descr)]) =>
  t;

let run:
  ([< | `Write(Unix.file_descr)], [< | `Read(Unix.file_descr)], t) => unit;
