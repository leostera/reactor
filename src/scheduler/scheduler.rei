module Policy: {
  type t;
  let default: unit => t;
};

let setup: Policy.t => unit;

let run: unit => unit;

let send: (Model.Pid.t('m), 'm) => unit;

let spawn: (Model.Process.task('a, 'm), 'a) => Model.Pid.t('m);
