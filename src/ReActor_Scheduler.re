open ReActor_Process;
open ReActor_Utils;

type sid = (string, string);

type t = {
  id: sid,
  processes: list(ReActor_Process.t),
  process_count: int,
  tracer: option(ReActor_Tracer.t),
};

let nextPid: t => Pid.t =
  ({id: (node_name, scheduler_id), process_count}) =>
    Pid.make(node_name, scheduler_id, process_count + 1);

let leastBusy: list(ref(t)) => ref(t) = workers => List.nth(workers, 0);

let pidToSid: Pid.t => sid =
  ((node_name, scheduler_id, _)) => (node_name, scheduler_id);

let findById: (sid, list(ref(t))) => ref(t) =
  i => List.find(s => s^.id == i);

let make: string => t =
  node_name => {
    id: (node_name, Random.shortId()),
    processes: [],
    process_count: 0,
    tracer: None,
  };

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

let exit: (Pid.t, ref(t)) => unit =
  (pid, scheduler) =>
    scheduler^.processes
    |> List.find(p => p.pid == pid)
    |> ReActor_Process.markAsDead;

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

let trace: (ReActor_Tracer.t, ref(t)) => unit =
  (tracer, scheduler) => {
    let time = tracer.timeout;
    Js.log({j|[ReActor] Tracer setup for $time ms.|j});
    scheduler := {...scheduler^, tracer: Some(tracer)};
    FFI_Runtime.defer(
      () => {
        scheduler := {...scheduler^, tracer: None};
        Js.log({j|[ReActor] Tracer stopped after $time ms.|j});
      },
      tracer.timeout,
    );
  };
