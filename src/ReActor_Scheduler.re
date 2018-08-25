open ReActor_Process;
open ReActor_Utils;

type t = {
  id: (string, string),
  process_count: int,
};

let nextPid = ({id: (node_name, scheduler_id), process_count}) =>
  Pid.make(node_name, scheduler_id, process_count + 1);

/* TODO(@ostera): pick the one with lowest process_count */
let leastBusy: list(ref(t)) => ref(t) = workers => List.nth(workers, 0);

let make = node_name => {
  id: (node_name, Random.shortId()),
  process_count: 0,
};

let spawn: (f('a), 'a, ref(t)) => Pid.t =
  (f, args, scheduler) => {
    let pid = scheduler^ |> nextPid;

    /* TODO(@ostera): save this process in the scheduler */
    let _ = ReActor_Process.make(pid, f, args);

    let scheduler' = {
      ...scheduler^,
      process_count: scheduler^.process_count + 1,
    };
    scheduler := scheduler';
    pid;
  };
