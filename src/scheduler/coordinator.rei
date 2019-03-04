/**
  The type of the global coordinator
  */
type t;

let setup: Policy.t => unit;

let run: unit => unit;

let current: unit => t;

module Tasks: {
  let send_message: (~pid: Model.Pid.t, ~msg: Model.Message.t) => unit;
  let spawn: (~task: Model.Process.task('a), ~state: 'a) => Model.Pid.t;
  let halt: unit => unit;
};
