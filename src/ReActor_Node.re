open ReActor_Process;
open ReActor_Utils;

type t = {
  node_name: string,
  schedulers: list(ref(ReActor_Scheduler.t)),
};

let make = () => {
  let node_name = Random.shortId();
  {node_name, schedulers: [ref(ReActor_Scheduler.make(node_name))]};
};

let spawn: (t, f('a), 'a) => Pid.t =
  (node, f, args) =>
    node.schedulers
    |> ReActor_Scheduler.leastBusy
    |> ReActor_Scheduler.spawn(f, args);
