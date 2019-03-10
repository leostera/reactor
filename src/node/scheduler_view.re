type t = {
  id: int,
  unix_pid: int,
  pipe_to_scheduler: Unix.file_descr,
  pipe_from_scheduler: Unix.file_descr,
  last_pid: Process.Pid.t,
  process_count: int,
};

let make = (`Child_pid(pid), `Write(to_scheduler), `Read(from_scheduler)) => {
  id: pid,
  unix_pid: pid,
  pipe_to_scheduler: to_scheduler,
  pipe_from_scheduler: from_scheduler,
  last_pid: Process.Pid.make(0, pid, 0),
  process_count: 0,
};

let id = t => t.id;

let unix_pid = t => t.unix_pid;

let last_pid = t => t.last_pid;

let next_pid = scheduler => {
  let pid = scheduler.last_pid |> Process.Pid.next;
  (
    {...scheduler, last_pid: pid, process_count: scheduler.process_count + 1},
    pid,
  );
};

module Registry:
  Reactor_registry.Registry.REGISTRY with type key = Int32.t and type value = t =
  Reactor_registry.Registry.Make({
    type key = Int32.t;
    let equal = Int32.equal;
    let hash = Hashtbl.hash;
    type value = t;
  });

let pipes = t => (
  `Read(t.pipe_from_scheduler),
  `Write(t.pipe_to_scheduler),
);

let fold_pipes = ((reads, writes), (read, write)) => {
  ([read, ...reads], [write, ...writes]);
};

let least_busy = schedulers => {
  let by_process_count = (w1, w2) => w1.process_count - w2.process_count;
  let sorted =
    schedulers
    |> Registry.values
    |> List.of_seq
    |> List.sort(by_process_count);
  switch (sorted) {
  | [hd, ..._] => Some(hd)
  | _ => None
  };
};
