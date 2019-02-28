type t;

let id: t => int;

let start: unit => option(t);

let least_busy: list(t) => option(t);

let last_pid: t => Model.Pid.t;

let next_pid: t => (t, Model.Pid.t);

let wait_next_available:
  Seq.t(t) => [> | `Receive(Seq.t(Bytecode.t)) | `Send(Seq.t(t)) | `Wait];

let send_task: (Bytecode.t, t) => unit;

module Child: {
  type task = [
    | `From_coordinator(Bytecode.t)
    | `From_worker(Bytecode.t)
    | `Reduction(unit => unit)
  ];

  type t = {
    id: int,
    unix_pid: int,
    pipe_to_coordinator: Unix.file_descr,
    pipe_from_coordinator: Unix.file_descr,
    last_pid: ref(Model.Pid.t),
    processes: Model.Registry.t,
    process_count: int,
    tasks: Task_queue.t(task),
  };

  let current: unit => option(t);
};
