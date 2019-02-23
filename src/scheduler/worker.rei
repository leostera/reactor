type t;

let id: t => int;

let start: unit => option(t);

let least_busy: list(t) => option(t);

let last_pid: t => Model.Pid.t;

let next_pid: t => (t, Model.Pid.t);

let wait_next_available:
  Seq.t(t) => [> | `Receive(Seq.t(Bytecode.t)) | `Send(Seq.t(t)) | `Wait];

let send_task: (Bytecode.t, t) => unit;
