module Message: {
  /**
    Messages in Reactor are typed. That means that for every message that you
    send, and receive, there will be an associated data constructor to ensure
    that messages are both well-formed and either handled or explicitly ignore.
    */
  type t = ..;
};

module Pid: {
  type t;

  let make: int => t;

  let to_string: t => string;

  let equal: (t, t) => bool;

  let next: t => t;
};

module Process: {
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
};

module Registry: {
  type t;

  let create: unit => t;

  let find: (t, Pid.t) => option(Process.t);

  let register: (t, Pid.t, Process.t) => t;

  let unregister: (t, Pid.t) => t;
};

module Scheduler: {
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
};
