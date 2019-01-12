type behavior('s) = [ | `Become('s) | `Terminate];

type env('s) = {self: unit => Pid.t};

type task('s) = (env('s), 's) => behavior('s);

type t;

let send: (Message.t, t) => unit;

let make: Pid.t => t;
