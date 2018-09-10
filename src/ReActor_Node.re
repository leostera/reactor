/** A Node maps directly to a Browser Agent.

  It is the orchestrator of schedulers, making sure that they are sharing the
  load fairly equally, and gives access to a name register and tracing
  functionality.

  There should only be a {i single node} per Browser Agent.

  See {!ReActor_Scheduler}, {!ReActor_Tracer}.
 */
type __doc; /* TODO(@ostera): Remove when
               https://github.com/facebook/reason/issues/2177 is fixed */

open ReActor_Process;
open ReActor_Utils;
open ReActor_Runtime;

/**
  Process Registry type.

  A simple Property-list keyed by a String and valued by a
  {!ReActor_Process.Pid.t}.
  */
/* TODO(@ostera): Make an instance of String.Map for this */
type proc_registry = list((string, Pid.t));

/** Node type.

  This internal record represents a Browser Agent. All operations in this module
  work on a value of this type.
 */
type t = {
  /** The name of the node as a string, usually looking like a SHA1. */
  node_name: string,
  /** A list of scheduler references. */
  schedulers: list(ref(ReActor_Scheduler.t)),
  /** A list of tuples used to keep track of registered names and their pids. */
  registry: ref(proc_registry),
};

/**
  Call this function to create a {!ReActor_Node.t}.

  By default a node will create as many schedulers as processors the Browser
  Agent has available.

  This means that a computer with 4 cores can use 4 separate execution threads,
  and thus will have 4 schedulers running in parallel.

  At least one of those schedulers will run on the Main Thread (Rendering
  Thread).
 */
let make: unit => t =
  () => {
    let node_name = Random.shortId();
    {
      node_name,
      schedulers: [
        ref(ReActor_Scheduler.make(node_name)),
        ...Array.make(hardwareConcurrency - 1, 0)
           |> Array.to_list
           |> List.map(_i => ReActor_Scheduler.make(node_name))
           |> List.map(ref),
      ],
      registry: ref([]),
    };
  };

/**
  Set up a {!ReActor_Tracer.t} on this node.
  */
let trace: (t, ReActor_Tracer.t) => unit =
  (node, tracer) =>
    node.schedulers |> List.iter(ReActor_Scheduler.trace(tracer));

/* TODO(@ostera): Rename this to `spawnInMain` since we can only spawn
   functions directly on the Main Thread. */
let spawn: (t, f('s), 's) => Pid.t =
  (node, f, args) =>
    node.schedulers
    |> ReActor_Scheduler.leastBusy
    |> ReActor_Scheduler.spawn(f, args);

let exit: (t, Pid.t) => unit =
  (node, pid) =>
    node.schedulers
    |> ReActor_Scheduler.findById(ReActor_Scheduler.pidToSid(pid))
    |> ReActor_Scheduler.exit(pid);

let register: (t, string, Pid.t) => Pid.t =
  (node, name, pid) => {
    node.registry := [(name, pid), ...node.registry^];
    pid;
  };

/**
  Lookup the [name] in the [node] registry. Returns a [Pid.t] if it found one.
 */
let whereIs: (t, string) => option(Pid.t) =
  (node, name) => {
    let byName = List.find(((name', _pid)) => name == name');
    switch (node.registry^ |> byName) {
    | (_name, pid) => Some(pid)
    | exception Not_found => None
    };
  };

/**
  Find the [process] to send a [message] to by [pid], given that the [pid]
  includes the identifier of the [scheduler] it runs on.

  Then write the [message] to the [process] mailbox.
 */
let send: (t, Pid.t, Message.t) => unit =
  (node, pid, message) =>
    node.schedulers
    |> ReActor_Scheduler.(findById(pidToSid(pid)))
    |> ReActor_Scheduler.send(pid, message);
