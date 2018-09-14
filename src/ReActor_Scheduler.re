open ReActor_Process;
open ReActor_Utils;
open ReActor_Tracer;

module Sid = {
  type t = (string, string);
  let make = node_name => (node_name, Random.shortId());
  let toString = ((node_name, scheduler_id)) => {j|<$node_name.$scheduler_id>|j};
};

type t = {
  id: Sid.t,
  processes: list(ReActor_Process.t),
  process_count: int,
  tracer: option(ReActor_Tracer.t),
};

let nextPid = ({id: (node_name, scheduler_id), process_count}) =>
  Pid.make(node_name, scheduler_id, process_count + 1);

let byProcessCount = (a, b) => compare(a^.process_count, b^.process_count);

let leastBusy = workers => workers |> List.sort(byProcessCount) |> List.hd;

let pidToSid = ((node_name, scheduler_id, _)) => (node_name, scheduler_id);

let findById = i => List.find(s => s^.id == i);

let make = node_name => {
  id: Sid.make(node_name),
  processes: [],
  process_count: 0,
  tracer: None,
};

let spawn = (f, args, scheduler) => {
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

let send = (pid, msg, scheduler) => {
  switch (scheduler^.tracer) {
  | Some(tracer) => ReActor_Tracer.trace(tracer, pid, msg)
  | None => ()
  };
  scheduler^.processes
  |> List.find(p => p.pid == pid)
  |> ReActor_Process.send(msg);
};

let trace = (tracer, scheduler) => {
  let timeout = tracer.timeout;
  Js.log({j|[ReActor] Tracer setup for $timeout  ms.|j});
  scheduler := {...scheduler^, tracer: Some(tracer)};
  ReActor_Runtime.defer(
    () => {
      scheduler := {...scheduler^, tracer: None};
      Js.log({j|[ReActor] Tracer stopped after $timeout ms.|j});
    },
    timeout,
  );
};
