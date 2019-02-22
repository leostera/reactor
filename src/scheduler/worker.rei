type t;

let id: t => int;

let start: unit => option(t);

let wait_next_available:
  Seq.t(t) => [ | `Receive(Seq.t(Bytecode.t)) | `Send(Seq.t(t)) | `Wait];
