type t;

let create: unit => t;

let find: (t, Process.Pid.t('m)) => option(Process.Process.t('m));

let register: (t, Process.Pid.t('m), Process.Process.t('m)) => unit;

let unregister: (t, Process.Pid.t('m)) => unit;

let length: t => int;
