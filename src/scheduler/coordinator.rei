/**
  The type of the global coordinator
  */
type t;

let setup: Policy.t => unit;

let run: unit => unit;

let current: unit => t;

module Tasks: {
  let send_message: (t, ~pid: Model.Pid.t('message), ~msg: 'message) => unit;
  let spawn:
    (t, ~task: Model.Process.task('a, 'message), ~state: 'a) =>
    Model.Pid.t('message);
};
