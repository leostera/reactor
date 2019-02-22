module Unix = UnixLabels;

type t = {
  policy: Policy.t,
  workers: Worker_registry.t,
};

let __global_coordinator: ref(t) =
  ref({policy: Policy.default(), workers: Worker_registry.create()});

let current = () => __global_coordinator^;

let configure = policy => {
  __global_coordinator := {...__global_coordinator^, policy};
};

let loop = workers => {
  Logs.app(m => m("Beginning Scheduling loop..."));

  let read_fds =
    Worker_registry.workers(workers)
    |> Seq.fold_left(
         (fds, w) => {
           let (_, `From_worker(fd)) = w |> Worker.pipes;
           [fd, ...fds];
         },
         [],
       );

  let rec work = () => {
    switch (Unix.select(~read=read_fds, ~write=[], ~except=[], ~timeout=-1.0)) {
    | (ins, _, _) when List.length(ins) > 0 =>
      Logs.debug(m => m("Select awoke!"));
      ins
      |> List.iter(fd => {
           let ic = Unix.in_channel_of_descr(fd);
           Logs.debug(m => m("Opened in_channel...\n%!"));
           let str: string = Marshal.from_channel(ic);
           Logs.debug(m => m("Read: %s\n%!", str));
         });
      work();
    | _ => work()
    };
  };

  work();
};

let start = () => {
  let {policy, workers} = current();

  Array.make(Policy.worker_count(policy), None)
  |> Array.iteri((_, _) =>
       switch (Worker.start()) {
       | Some(worker) =>
         let worker_id = worker |> Worker.id |> Int32.of_int;
         Worker_registry.register(workers, worker_id, worker) |> ignore;
       | None => ()
       }
     );

  Logs.app(m =>
    m(
      "Spawned %d out of %d workers: %s",
      policy |> Policy.worker_count,
      workers |> Worker_registry.size,
      workers
      |> Worker_registry.ids
      |> Seq.map(Int32.to_string)
      |> List.of_seq
      |> String.concat(", "),
    )
  );

  loop(workers);
};

let worker_for_pid = (coordinator, ~pid) => {
  let {worker_id, _}: Model.Pid.view = Model.Pid.view(pid);
  Worker_registry.find(coordinator.workers, worker_id);
};
