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
};
