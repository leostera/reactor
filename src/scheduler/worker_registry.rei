type t;

let create: unit => t;

let find: (t, Int32.t) => option(Worker.t);

let register: (t, Int32.t, Worker.t) => t;

let unregister: (t, Int32.t) => t;

let size: t => int;

let ids: t => Seq.t(Int32.t);

let workers: t => Seq.t(Worker.t);
