module rec Process: {
  type behavior('s, 'm) = [ | `Become('s) | `Terminate];

  type env('s, 'm) = {
    self: unit => Pid.t('m),
    recv: unit => option('m),
  };

  type task('s, 'm) = (env('s, 'm), 's) => behavior('s, 'm);

  type t('m);

  let send: (t('m), 'm) => unit;

  let recv: (t('m), unit) => option('m);

  let make: Pid.t('m) => t('m);
} = {
  type behavior('s, 'm) = [ | `Become('s) | `Terminate];

  type env('s, 'm) = {
    self: unit => Pid.t('m),
    recv: unit => option('m),
  };

  type task('s, 'm) = (env('s, 'm), 's) => behavior('s, 'm);

  type t('m) = {
    pid: Pid.t('m),
    mailbox: Mailbox.t('m),
  };

  let send = (proc, msg) => Mailbox.send(proc.mailbox, msg);

  let recv = (proc, ()) => Mailbox.recv(proc.mailbox);

  let make = pid => {pid, mailbox: Mailbox.create()};
}
and Pid: {
  type t('m);

  type view = {
    node_id: int32,
    worker_id: int32,
    process_id: int32,
  };

  let make: (int, int, int) => t('m);

  let view: t('m) => view;

  let to_string: t('m) => string;

  let equal: (t('m), t('m)) => bool;

  let next: t('m) => t('n);

  let to_t: view => t('m);

  let compare: (t('m), t('n)) => [ | `Eq | `Gt | `Lt | `Incomparable];

  let unique_key: t('m) => Registry_key.t(Process.t('m));
} = {
  type t('m) = (int, int, int, Registry_key.t(Process.t('m)));

  type view = {
    node_id: int32,
    worker_id: int32,
    process_id: int32,
  };

  let make = (node, worker, process) => (
    node,
    worker,
    process,
    Registry_key.create(),
  );

  let view = ((n, w, p, _)) => {
    node_id: n |> Int32.of_int,
    worker_id: w |> Int32.of_int,
    process_id: p |> Int32.of_int,
  };

  let to_string = ((n, w, p, _)) => Printf.sprintf("<%d,%d,%d>", n, w, p);

  let equal = ((n1, w1, p1, _), (n2, w2, p2, _)) =>
    n1 == n2 && w1 == w2 && p1 == p2;

  let next = ((n, w, p, _)) => (n, w, p + 1, Registry_key.create());

  let unique_key = ((_, _, _, k)) => k;

  let to_t = ({node_id, worker_id, process_id}) =>
    make(
      node_id |> Int32.to_int,
      worker_id |> Int32.to_int,
      process_id |> Int32.to_int,
    );

  let compare = ((n1, w1, p1, _), (n2, w2, p2, _)) => {
    switch (n1 == n2, w1 == w2, p1, p2) {
    | (true, true, p1, p2) when p1 == p2 => `Eq
    | (true, true, p1, p2) when p1 > p2 => `Gt
    | (true, true, p1, p2) when p1 < p2 => `Lt
    | _ => `Incomparable
    };
  };
};
