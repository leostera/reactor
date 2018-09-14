/**

  A Scheduler maps directly to an Event Loop / Execution Thread in the Browser
  Agent.

  It is piece in charge of execution scheduled computations that make up the
  processes.

  */;

module Sid: {
  /**
    A Scheduler Identifier, or [Sid], is a tuple of the [ReActor_Node] it
    belongs to, and it's own unique string identifier.
  */
  type t = (string, string);
  let make: string => t;
  let toString: t => string;
};

/** Scheduler type */
type t = {
  /** The schedulers identifier */
  id: Sid.t,
  /** A list of processes currently being executed in this scheduler */
  processes: list(ReActor_Process.t),
  /** The number of processes this scheduler has ever executed */
  process_count: int,
  /** Optional tracer to inspect messages */
  tracer: option(ReActor_Tracer.t),
};

let nextPid: t => ReActor_Process.Pid.t;

let byProcessCount: (ref(t), ref(t)) => int;

let leastBusy: list(ref(t)) => ref(t);

let pidToSid: ReActor_Process.Pid.t => Sid.t;

let findById: (Sid.t, list(ref(t))) => ref(t);

/**
  Create an empty scheduler given a [node_name].

  Each scheduler will have a randomized 7-char hex id.
  */
let make: string => t;

/**
  Create a processes by calculating the next available [pid], spawning it, and
  allocating it in the [scheduler] list of processes.

  Returns the newly allocated [pid].
  */
let spawn: (ReActor_Process.f('s), 's, ref(t)) => ReActor_Process.Pid.t;

/**
  Marks a process as Dead, so it won't continue executing on it's next scheduled
  computation.
  */
let exit: (ReActor_Process.Pid.t, ref(t)) => unit;

/**
  Finds a process by [pid] in the list of processes of the [scheduler] and
  writes the [message] to it's mailbox.

  If a [tracer] has been enabled, executes it's [trace] function before.
  */
let send: (ReActor_Process.Pid.t, ReActor_Process.Message.t, ref(t)) => unit;

/**
  Sets up a [tracer] in the given [scheduler].

  Warning: tracing can be incredibly expensive because _every message_ that goes
  through a [scheduler] will be checked for tracing before being delivered. Use
  with care.

  A [tracer] will be scheduled for removal in [timeout] ms.
  */
let trace: (ReActor_Tracer.t, ref(t)) => unit;
