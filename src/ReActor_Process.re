module Message = {
  type t = ..;
};

module Pid = {
  type t = (string, string, int);
  let make: (string, string, int) => t =
    (node_name, scheduler_id, process_number) => (
      node_name,
      scheduler_id,
      process_number,
    );
  let toString: t => string =
    ((node_name, scheduler_id, process_number)) => {j|<$node_name.$scheduler_id.$process_number>|j};
};

module Status = {
  type t =
    | Alive
    | Dead;

  let toString: t => string =
    fun
    | Alive => "alive"
    | Dead => "dead";
};

type env('s) = {
  loop: 's => 's,
  self: unit => Pid.t,
  sleep: (int, unit => 's) => unit,
  recv: (Message.t => 's) => unit,
};

type f('s) = (env('s), 's) => 's;

type t = {
  pid: Pid.t,
  status: ref(Status.t),
  mailbox: ref(list(Message.t)),
};

let markAsDead: t => unit = p => p.status := Dead;

let keepAlive: t => unit = p => p.status := Alive;

let send: ('m, t) => unit =
  (msg, process) => {
    process.mailbox := List.append(process.mailbox^, [msg]);
    ();
  };

let rec recv: (t, Message.t => 's) => unit =
  (process, f) =>
    FFI_Runtime.nextTick(() =>
      switch (process.mailbox^) {
      | [] => recv(process, f)
      | [message, ...mailbox'] =>
        let _ = f(message);
        process.mailbox := mailbox';
      }
    );

let make: (Pid.t, f('s), 's) => t =
  (pid, f, initial_args) => {
    let process = {pid, status: ref(Status.Alive), mailbox: ref([])};
    let rec run = args =>
      switch (process.status^) {
      | Alive =>
        let _ = f(env, args);
        ();
      | Dead => ()
      }
    and env = {
      self: () => pid,
      recv: recv(process),
      sleep: (ms, g) => FFI_Runtime.defer(() => g(), ms),
      loop: args => {
        FFI_Runtime.nextTick(() => run(args));
        args;
      },
    };
    let _ = run(initial_args);
    process;
  };
