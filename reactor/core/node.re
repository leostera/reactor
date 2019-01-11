/* TODO(@ostera): Make an instance of StringMap for this */
type proc_registry = list((string, Process.Pid.t));

type t = {
  node_name: string,
  schedulers: list(ref(Scheduler.t)),
  registry: ref(proc_registry),
};

let make = (~scheduler_count) => {
  let node_name = Random.int(1000) |> string_of_int;
  {
    node_name,
    schedulers: [
      ref(Scheduler.make(node_name)),
      ...Array.make(scheduler_count - 1, 0)
         |> Array.to_list
         |> List.map(_i => Scheduler.make(node_name))
         |> List.map(ref),
    ],
    registry: ref([]),
  };
};

let trace = (node, tracer) =>
  node.schedulers |> List.iter(Scheduler.trace(tracer));

/* TODO(@ostera): Rename this to `spawnInMain` since we can only spawn
   functions directly on the Main Thread */
let spawn = (node, f, args) =>
  node.schedulers |> Scheduler.leastBusy |> Scheduler.spawn(f, args);

let exit = (node, pid) =>
  node.schedulers
  |> Scheduler.findById(Scheduler.pidToSid(pid))
  |> Scheduler.exit(pid);

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
  |> Scheduler.(findById(pidToSid(pid)))
  |> Scheduler.send(pid, message);
