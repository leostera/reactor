/** A Node maps directly to a Browser Agent

  It is the orchestrator of schedulers, making sure that they are sharing the
  load fairly equally, and gives access to a name register and tracing
  functionality

  There should only be a {i single node} per Browser Agent

  See {!Scheduler}, {!Tracer}
 */;

/**
  Process.t Registry type

  A simple Property-list keyed by a String and valued by a
  {!Process.Pid.t}
  */
type proc_registry;

/** Node type

  This internal record represents a Browser Agent All operations in this module
  work on a value of this type
 */
type t = {
  /** The name of the node as a string, usually looking like a SHA1 */
  node_name: string,
  /** A list of scheduler references */
  schedulers: list(ref(Scheduler.t)),
  /** A list of tuples used to keep track of registered names and their pids */
  registry: ref(proc_registry),
};

/**
  Call this function to create a {!Nodet}

  By default a node will create as many schedulers as processors the Browser
  Agent has available

  This means that a computer with 4 cores can use 4 separate execution threads,
  and thus will have 4 schedulers running in parallel

  At least one of those schedulers will run on the Main Thread (Rendering
  Thread)
 */
let make: (~scheduler_count: int) => t;

/**
  Set up a {!Tracer.t} on this node
  */
let trace: (t, Tracer.t) => unit;

let spawn: (t, Process.f('s), 's) => Process.Pid.t;

let exit: (t, Process.Pid.t) => unit;

let register: (t, string, Process.Pid.t) => Process.Pid.t;

/**
  Lookup the [name] in the [node] registry Returns a [Pid.t] if it found one
 */
let where_is: (t, string) => option(Process.Pid.t);

/**
  Find the [process] to send a [message] to by [pid], given that the [pid]
  includes the identifier of the [scheduler] it runs on

  Then write the [message] to the [process] mailbox
 */
let send: (t, Process.Pid.t, Process.Message.t) => unit;
