let send: (Process.Pid.t, string) => unit;

let (<-): (Process.Pid.t, string) => unit;

let spawn: (Process.task('a), 'a) => Process.Pid.t;

let halt: unit => unit;

let exit: unit => [> | `Terminate];
