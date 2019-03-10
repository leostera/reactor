let send: (Reactor_process.Process.Pid.t, string) => unit;

let (<-): (Reactor_process.Process.Pid.t, string) => unit;

let spawn:
  (Reactor_process.Process.task('a), 'a) => Reactor_process.Process.Pid.t;

let halt: unit => unit;

let exit: unit => [> | `Terminate];
