module Pid = {
  type t = (string, string, int);
  let make = (node_name, scheduler_id, process_number) => (
    node_name,
    scheduler_id,
    process_number,
  );
};

type status =
  | Alive
  | Dead;

type env('a) = {
  self: unit => Pid.t,
  loop: 'a => 'a,
};

type f('a) = (env('a), 'a) => 'a;

type t('a) = {
  pid: Pid.t,
  f: f('a),
  initial_args: 'a,
  status: ref(status),
};

let make = (pid, f, initial_args) => {
  let process = {pid, f, initial_args, status: ref(Dead)};
  let rec run = args => {
    let args' = f(env, args);
    switch (process.status^) {
    | Alive => FFI_Runtime.defer(() => run(args'), 0)
    | Dead => ()
    };
  }
  and env = {
    self: () => pid,
    loop: args => {
      FFI_Runtime.defer(() => run(args), 0);
      args;
    },
  };
  let _ = run(initial_args);
  process;
};
