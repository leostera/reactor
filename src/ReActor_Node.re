open ReActor_Process;
open ReActor_Utils;
open FFI_Runtime;

type t = {
  node_name: string,
  schedulers: list(ref(ReActor_Scheduler.t)),
  registry: ref(list((string, Pid.t))),
};

let make: unit => t =
  () => {
    let node_name = Random.shortId();
    {
      node_name,
      schedulers: [
        ref(ReActor_Scheduler.make(node_name)),
        ...Array.make(hardwareConcurrency - 2, 0)
           |> Array.to_list
           |> List.map(_i => ReActor_Scheduler.make(node_name))
           |> List.map(ref),
      ],
      registry: ref([]),
    };
  };

let trace: (t, ReActor_Tracer.t) => unit =
  (node, tracer) =>
    node.schedulers |> List.iter(ReActor_Scheduler.trace(tracer));

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

let where_is: (t, string) => option(Pid.t) =
  (node, name) => {
    let byName = List.find(((name', _pid)) => name == name');
    switch (node.registry^ |> byName) {
    | (_name, pid) => Some(pid)
    | exception Not_found => None
    };
  };

let send: (t, Pid.t, Message.t) => unit =
  (node, pid, msg) =>
    node.schedulers
    |> ReActor_Scheduler.(findById(pidToSid(pid)))
    |> ReActor_Scheduler.send(pid, msg);
