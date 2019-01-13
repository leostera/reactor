type t;

module type BASE = {
  let defer: (unit => unit) => unit;
  let run: unit => unit;
};

module type SCHEDULER = {
  let make: unit => t;
  let spawn: (t, Process.task('state), 'state) => Pid.t;
  let run: unit => unit;
  let send: (t, Pid.t, Message.t) => unit;
};

module Make: (M: BASE) => SCHEDULER;
