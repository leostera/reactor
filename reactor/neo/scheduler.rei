type t;

module type BASE = {
  let defer: (unit => unit) => unit;
  let run: t => unit;
};

module type SCHEDULER = {
  let make: unit => t;
  let spawn: (t, Process.task('state), 'state) => Pid.t;
  let run: t => unit;
};

module Make: (M: BASE) => SCHEDULER;
