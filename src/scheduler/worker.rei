type t;

let id: t => int;

let start: unit => option(t);

let pipes:
  t =>
  ([ | `To_worker(Unix.file_descr)], [ | `From_worker(Unix.file_descr)]);

let enqueue_message_for_pid:
  (t, ~pid: Model.Pid.t, ~msg: Model.Message.t) => unit;
