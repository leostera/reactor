type behavior('s) = [ | `Become('s) | `Terminate];

type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(Message.t),
};

type task('s) = (env('s), 's) => behavior('s);

type t;

let send: (t, Message.t) => unit;

let recv: (t, unit) => option(Message.t);

let make: Pid.t => t;
