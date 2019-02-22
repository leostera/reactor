type t;

type view = {
  node_id: int32,
  worker_id: int32,
  process_id: int32,
};

let make: (int, int, int) => t;

let view: t => view;

let to_string: t => string;

let equal: (t, t) => bool;

let next: t => t;
