type t;

let create: unit => t;

let find: (t, Pid.t) => option(Process.t);

let register: (t, Pid.t, Process.t) => t;

let unregister: (t, Pid.t) => t;
