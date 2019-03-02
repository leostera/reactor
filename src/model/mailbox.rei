type t;

let send: (t, Message.t) => unit;

let recv: t => option(Message.t);

let create: unit => t;