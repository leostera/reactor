module Message: {type t = string;};

module Pid: {
  type t;

  type view = {
    node_id: int32,
    scheduler_id: int32,
    process_id: int32,
  };

  let make: (int, int, int) => t;

  let view: t => view;

  let to_string: t => string;

  let equal: (t, t) => bool;

  let next: t => t;

  let hash: t => int;
};

type behavior('s) = [ | `Become('s) | `Terminate | `Defer(Lwt.t('s))];

type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(string),
};

type task('s) = (env('s), 's) => behavior('s);

type t;

let send: (t, Message.t) => unit;

let recv: (t, unit) => option(string);

let make: Pid.t => t;

module Registry:
  Reactor_registry.Registry.REGISTRY with type key = Pid.t and type value = t;
