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
  Logs.debug(m => m("Beginning Scheduling loop..."));

  let read_fds =
    Worker_registry.workers(workers)
    |> Seq.fold_left(
         (fds, w) => {
           let (_, `From_worker(fd)) = w |> Worker.pipes;
           [fd, ...fds];
         },
         [],
       );

  let rec sync = () => {
    switch (
      Platform.Process.select(
        ~read=read_fds,
        ~write=[],
        ~except=[],
        ~timeout=-1.0,
      )
    ) {
    | (`Read(ins), _, _) when List.length(ins) > 0 =>
      ins
      |> List.iter(fd => {
           let marshalled_instruction_size = 38;
           let raw =
             Platform.Process.read(~fd, ~len=marshalled_instruction_size);
           let str: string = Marshal.from_bytes(raw, 0);
           Logs.debug(m => m("Read: %s", str));
           ();
         });
      sync();
    | _ => sync()
    };
  };

  sync();
};

let start = () => {
  let {policy, workers} = current();

  Policy.worker_count(policy)
  |> (size => Array.make(size, None))
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
