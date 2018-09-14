open ReActor_Process;
open ReActor_Utils;
open ReActor_Runtime;

/* TODO(@ostera): Make an instance of String.Map for this */
type proc_registry = list((string, Pid.t));

type t = {
  node_name: string,
  schedulers: list(ref(ReActor_Scheduler.t)),
  registry: ref(proc_registry),
};

let make = () => {
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

let trace = (node, tracer) =>
  node.schedulers |> List.iter(ReActor_Scheduler.trace(tracer));

/* TODO(@ostera): Rename this to `spawnInMain` since we can only spawn
   functions directly on the Main Thread. */
let spawn = (node, f, args) =>
  node.schedulers
  |> ReActor_Scheduler.leastBusy
  |> ReActor_Scheduler.spawn(f, args);

let exit = (node, pid) =>
  node.schedulers
  |> ReActor_Scheduler.findById(ReActor_Scheduler.pidToSid(pid))
  |> ReActor_Scheduler.exit(pid);

let register = (node, name, pid) => {
  node.registry := [(name, pid), ...node.registry^];
  pid;
};

let whereIs = (node, name) => {
  let byName = List.find(((name', _pid)) => name == name');
  switch (node.registry^ |> byName) {
  | (_name, pid) => Some(pid)
  | exception Not_found => None
  };
};

let send = (node, pid, message) =>
  node.schedulers
  |> ReActor_Scheduler.(findById(pidToSid(pid)))
  |> ReActor_Scheduler.send(pid, message);
