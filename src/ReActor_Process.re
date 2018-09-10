/**

  A Process maps directly to a unit of computation, possibly long-lived,
  carrying out tasks for a particular purpose.

  If you are familiar with the Actor-model, a Process is what the Erlang/Elixir
  folk call an Actor.

  */
open ReActor_Runtime;

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
  The Behavior type defines how a given Actor will behave as soon as the
  current computation is completed.

  It is in fact a value of this type that the computation {i must} return.

  The literature tends to uses the name {!Become} to indicate that an actor
  will {i somehow} turn into a new actor, with the exact same identifier and
  mailbox, but that will begin it's execution with a different set of inputs.

  In essence, if an actor's main function looks like:

  {[
    let proc: Process.f(int) = (_, counter) => Become(counter + 1)
  ]}

  Then this actor will increment it's internal counter by 1 on every scheduled
  execution, and it will continue doing so for as long as the system is running.

  A slightly different actor:

  {[
    let proc: Process.f(int) = (_, counter) => Suspend(counter, counter+1)
  ]}

  Will be indefinitely scheduled (like the one above) and wait {!counter}
  milliseconds before becoming a new actor with an incremented counter.
  */
type behavior('s) =
  | /**
      Indicate that this actor should not become a new actor but instead
      should be terminated as soon as the current computation completes.
     */
    Terminate
  | /**
      Indicate that this actor should become a new actor with the same
      identifier and mailbox, but with a new state {!'s}.
     */
    Become(
      's,
    )
  | /**
      Indicate this actor should become a new actor with new state {!'s} in {i
      at most} [n] milliseconds.
      */
    Suspend(
      int,
      's,
    )
  | /**
      Indicate that this actor should {!Become} as soon as the next Animation
      Frame is available; this is particularly useful for rendering purposes.
     */
    OnAnimationFrame(
      's,
    );

/**
  The environment type.

  Each [process] gets access to an environment [env] that gives them the
  capability to

  {ul
    {- use [self] to access it's own [Pid.t] (useful for establishing
    communications). }
    {- Use [recv] to consume messages from their mailbox. }}

  The semantics of [recv] dictate that if the mailbox is **empty**, the process
  will await for a message to arrive before continuing.
  */
type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(Message.t),
};

/**
  The type of a [process] function. It receives an environment and a given
  state.
  */
type f('s) = (env('s), 's) => behavior('s);

/**
  The Process type.

  A process consists of a [mailbox], where the messages will be stored; a [pid]
  that identifies it uniquely among all the other processes in the system; and a
  [status] that indicates whether the process will continue executing or if it
  will stop executing after the current scheduled computation ends.
  */
type t = {
  /** The identifier for this process. */
  pid: Pid.t,
  /**
    Whether this process is currently Alive or if it has been naturally or
   abruptly Terminated.
   */
  status: ref(Status.t),
  /**
    The list of messages associated to this {!Pid.t}.
    */
  mailbox: ref(list(Message.t)),
};

let markAsDead: t => unit = p => p.status := Dead;

let keepAlive: t => unit = p => p.status := Alive;

let send: ('m, t) => unit =
  (msg, process) => {
    process.mailbox := List.append(process.mailbox^, [msg]);
    ();
  };

let recv: (t, unit) => option(Message.t) =
  (process, ()) =>
    switch (process.mailbox^) {
    | [] => None
    | [message, ...mailbox'] =>
      process.mailbox := mailbox';
      Some(message);
    };

/**
  Create a brand new process and begin executing it's main function.

  Given a [pid] and a function [f], a new [process] will be created with an
  empty [mailbox]; alongside it, an environment [env] will be created that will
  have access to the [pid] and the processes' [mailbox].

  A process is never recreated when using [loop], [recv], or [sleep], and it
  will continue to maintain it's identify ([pid]) until it is marked as [Dead].

  The [process] begins executing immediately before this function returns the
  [process] descriptor.
  */
let make: (Pid.t, f('s), 's) => t =
  (pid, f, initial_args) => {
    let process = {pid, status: ref(Status.Alive), mailbox: ref([])};
    let rec run = args =>
      switch (f(env, args)) {
      | exception ex =>
        Js.log({j|Process Terminated: $process threw $ex|j});
        process.status := Dead;
      | Terminate => process.status := Dead
      | OnAnimationFrame(newState) => onAnimationFrame(() => run(newState))
      | Suspend(delay, newState) => defer(() => run(newState), delay)
      | Become(newState) => nextTick(() => run(newState))
      }
    and env = {self: () => pid, recv: recv(process)};
    let _ = run(initial_args);
    process;
  };
