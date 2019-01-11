module Message = {
  type t = ..;
};

module Pid = {
  type t = (string, string, int);
  let make = (node_name, scheduler_id, process_number) => (
    node_name,
    scheduler_id,
    process_number,
  );
  let to_string = ((node_name, scheduler_id, process_number)) => {j|<$node_name$scheduler_id$process_number>|j};
};

module Status = {
  type t =
    | Alive
    | Dead;

  let to_string: t => string =
    fun
    | Alive => "alive"
    | Dead => "dead";
};

type behavior('s) =
  | Become('s)
  | Suspend(int, 's)
  | Terminate;

type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(Message.t),
};

type f('s) = (env('s), 's) => behavior('s);

type t = {
  pid: Pid.t,
  status: ref(Status.t),
  mailbox: ref(list(Message.t)),
};

let mark_as_dead = p => p.status := Dead;

let keep_alive = p => p.status := Alive;

let send = (msg, process) => {
  process.mailbox := List.append(process.mailbox^, [msg]);
  ();
};

let recv = (process, ()) =>
  switch (process.mailbox^) {
  | [] => None
  | [message, ...mailbox'] =>
    process.mailbox := mailbox';
    Some(message);
  };

let make = (pid, f, initial_args) => {
  let process = {pid, status: ref(Status.Alive), mailbox: ref([])};
  let rec run = args =>
    switch (f(env, args)) {
    | exception ex =>
      Logs.debug(m =>
        m(
          "Process Terminated: %s threw %s",
          process.pid |> Pid.to_string,
          ex |> Printexc.to_string,
        )
      );
      process.status := Dead;
    | Terminate => process.status := Dead
    | Suspend(delay, newState) =>
      /* defer(() => run(newState), delay) */
      ()
    | Become(newState) =>
      /* nextTick(() => run(newState)) */
      ()
    }
  and env = {self: () => pid, recv: recv(process)};
  let _ = run(initial_args);
  process;
};
