module Sid = {
  type t = (string, string);
  let make = node_name => (
    node_name,
    Random.int(1000000000) |> string_of_int,
  );
  let to_string = ((node_name, scheduler_id)) => {j|<$node_name$scheduler_id>|j};
};

type t = {
  id: Sid.t,
  processes: list(Process.t),
  process_count: int,
  tracer: option(Tracer.t),
};

let nextPid = ({id: (node_name, scheduler_id), process_count}) =>
  Process.Pid.make(node_name, scheduler_id, process_count + 1);

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

  let process = Process.make(pid, f, args);

  let scheduler' = {
    ...scheduler^,
    processes: [process, ...scheduler^.processes],
    process_count: scheduler^.process_count + 1,
  };
  scheduler := scheduler';

  pid;
};

let exit = (pid, scheduler) =>
  scheduler^.processes
  |> List.find(p => Process.(p.pid == pid))
  |> Process.markAsDead;

let send = (pid, msg, scheduler) => {
  switch (scheduler^.tracer) {
  | Some(tracer) => Tracer.trace(tracer, pid, msg)
  | None => ()
  };
  scheduler^.processes
  |> List.find(p => Process.(p.pid == pid))
  |> Process.send(msg);
};

let trace = (tracer, scheduler) => {
  let timeout = Tracer.(tracer.timeout);
  Logs.debug(m => m("[ReActor] Tracer setup for %d ms", timeout));
  scheduler := {...scheduler^, tracer: Some(tracer)};
  /*
   Runtimedefer(
     () => {
       scheduler := {...scheduler^, tracer: None};
       Logs.debug({j|[ReActor] Tracer. stopped after $timeout ms|j});
     },
     timeout,
   );
   */
  ();
};
