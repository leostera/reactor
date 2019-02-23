module Policy: {
  type t;
  let default: unit => t;
};

let setup: Policy.t => unit;

let run: unit => unit;

let send: (Model.Pid.t, Model.Message.t) => unit;

let spawn: (Model.Process.task('a), 'a) => Model.Pid.t;
