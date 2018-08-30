/**

  A Scheduler maps directly to an Event Loop / Execution Thread in the Browser
  Agent.

  It is piece in charge of execution scheduled computations that make up the
  processes.

  */
open ReActor_Process;
open ReActor_Utils;

module Sid = {
  /**
    A Scheduler Identifier, or [Sid], is a tuple of the [ReActor_Node] it
    belongs to, and it's own unique string identifier.
  */
  type t = (string, string);
  let make: string => t = node_name => (node_name, Random.shortId());
  let toString: t => string =
    ((node_name, scheduler_id)) => {j|<$node_name.$scheduler_id>|j};
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

let nextPid: t => Pid.t =
  ({id: (node_name, scheduler_id), process_count}) =>
    Pid.make(node_name, scheduler_id, process_count + 1);

let byProcessCount = (a, b) => compare(a^.process_count, b^.process_count);

let leastBusy: list(ref(t)) => ref(t) =
  workers => workers |> List.sort(byProcessCount) |> List.hd;

let pidToSid: Pid.t => Sid.t =
  ((node_name, scheduler_id, _)) => (node_name, scheduler_id);

let findById: (Sid.t, list(ref(t))) => ref(t) =
  i => List.find(s => s^.id == i);

/**
  Create an empty scheduler given a [node_name].

  Each scheduler will have a randomized 7-char hex id.
  */
let make: string => t =
  node_name => {
    id: Sid.make(node_name),
    processes: [],
    process_count: 0,
    tracer: None,
  };

/**
  Create a processes by calculating the next available [pid], spawning it, and
  allocating it in the [scheduler] list of processes.

  Returns the newly allocated [pid].
  */
let spawn: (f('s), 's, ref(t)) => Pid.t =
  (f, args, scheduler) => {
    let pid = scheduler^ |> nextPid;

    let process = ReActor_Process.make(pid, f, args);

    let scheduler' = {
      ...scheduler^,
      processes: [process, ...scheduler^.processes],
      process_count: scheduler^.process_count + 1,
    };
    scheduler := scheduler';

    pid;
  };

/**
  Marks a process as Dead, so it won't continue executing on it's next scheduled
  computation.
  */
let exit: (Pid.t, ref(t)) => unit =
  (pid, scheduler) =>
    scheduler^.processes
    |> List.find(p => p.pid == pid)
    |> ReActor_Process.markAsDead;

/**
  Finds a process by [pid] in the list of processes of the [scheduler] and
  writes the [message] to it's mailbox.

  If a [tracer] has been enabled, executes it's [trace] function before.
  */
let send: (Pid.t, Message.t, ref(t)) => unit =
  (pid, msg, scheduler) => {
    switch (scheduler^.tracer) {
    | Some(tracer) => ReActor_Tracer.trace(tracer, pid, msg)
    | None => ()
    };
    scheduler^.processes
    |> List.find(p => p.pid == pid)
    |> ReActor_Process.send(msg);
  };

/**
  Sets up a [tracer] in the given [scheduler].

  Warning: tracing can be incredibly expensive because _every message_ that goes
  through a [scheduler] will be checked for tracing before being delivered. Use
  with care.

  A [tracer] will be scheduled for removal in [timeout] ms.
  */
let trace: (ReActor_Tracer.t, ref(t)) => unit =
  (tracer, scheduler) => {
    let timeout = tracer.timeout;
    Js.log({j|[ReActor] Tracer setup for $timeout  ms.|j});
    scheduler := {...scheduler^, tracer: Some(tracer)};
    FFI_Runtime.defer(
      () => {
        scheduler := {...scheduler^, tracer: None};
        Js.log({j|[ReActor] Tracer stopped after $timeout ms.|j});
      },
      timeout,
    );
  };
