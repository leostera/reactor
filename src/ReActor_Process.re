/**

  A Process maps directly to a unit of computation, possibly long-lived,
  carrying out tasks for a particular purpose.

  If you are familiar with the Actor-model, a Process is what the Erlang/Elixir
  folk call an Actor.

  */
module Message = {
  /**
    The Message type. All messages within [ReActor] are data of this type.
    This is an open type, meaning any new process can define it's own set of
    variants representing the messages they can answer to.
    */
  type t = ..;
};

module Pid = {
  /**
    The process identifier type.

    It consists of a tuple of the [node_name], the [scheduler_id], and the
    [process_count] at the point of spawning.

    This means every given [Pid.t] is fully unique, but also indicates exactly
    where it's running and relatively how old it was (lower [process_count]
    indicate an older process).
    */
  type t = (string, string, int);
  let make: (string, string, int) => t =
    (node_name, scheduler_id, process_number) => (
      node_name,
      scheduler_id,
      process_number,
    );
  let toString: t => string =
    ((node_name, scheduler_id, process_number)) => {j|<$node_name.$scheduler_id.$process_number>|j};
};

module Status = {
  /**
    Status type indicating whether the process should continue execution or if
    it should be terminated.
    */
  type t =
    | Alive
    | Dead;

  let toString: t => string =
    fun
    | Alive => "alive"
    | Dead => "dead";
};

/**
  The environment type.

  Each [process] gets access to an environment [env] that gives them the
  capability to

  {ul
    {- [loop] indefinitely while making state changes immutably. }
    {- use [self] to access it's own [Pid.t] (useful for establishing
    communications). }
    {- [sleep] for a finite amount of time before continuing a computation. }
    {- Use [recv] to consume messages from their mailbox. }}

  The semantics of [recv] dictate that if the mailbox is **empty**, the process
  will await for a message to arrive before continuing.
  */
type env('s) = {
  loop: 's => 's,
  self: unit => Pid.t,
  sleep: (int, unit => 's) => unit,
  recv: (Message.t => 's) => unit,
};

/**
  The type of a [process] function. It receives an environment and a given
  state.
  */
type f('s) = (env('s), 's) => 's;

/**
  The Process type.

  A process consists of a [mailbox], where the messages will be stored; a [pid]
  that identifies it uniquely among all the other processes in the system; and a
  [status] that indicates whether the process will continue executing or if it
  will stop executing after the current scheduled computation ends.
  */
type t = {
  pid: Pid.t,
  status: ref(Status.t),
  mailbox: ref(list(Message.t)),
};

let markAsDead: t => unit = p => p.status := Dead;

let keepAlive: t => unit = p => p.status := Alive;

let send: ('m, t) => unit =
  (msg, process) => {
    process.mailbox := List.append(process.mailbox^, [msg]);
    ();
  };

let rec recv: (t, Message.t => 's) => unit =
  (process, f) =>
    FFI_Runtime.nextTick(() =>
      switch (process.mailbox^) {
      | [] => recv(process, f)
      | [message, ...mailbox'] =>
        let _ = f(message);
        process.mailbox := mailbox';
      }
    );

/**
  Create a brand new process and begin executing it's main function.

  Given a [pid] and a function [f], a new [process] will be created with an
  empty [mailbox]; alongside it, an environment [env] will be created that will
  have access to the [pid] and the [mailbox].

  A process is never recreated when using [loop], [recv], or [sleep], and it
  will continue to maintain it's identify ([pid]) until it is marked as [Dead].

  The [process] begins executing immediately before this function returns the
  [process] descriptor.
  */
let make: (Pid.t, f('s), 's) => t =
  (pid, f, initial_args) => {
    let process = {pid, status: ref(Status.Alive), mailbox: ref([])};
    let rec run = args =>
      switch (process.status^) {
      | Alive =>
        let _ = f(env, args);
        ();
      | Dead => ()
      }
    and env = {
      self: () => pid,
      recv: recv(process),
      sleep: (ms, g) => FFI_Runtime.defer(() => g(), ms),
      loop: args => {
        FFI_Runtime.nextTick(() => run(args));
        args;
      },
    };
    let _ = run(initial_args);
    process;
  };
