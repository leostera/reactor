type t = (int, int, int);

type view = {
  node_id: int32,
  worker_id: int32,
  process_id: int32,
};

let make = (node, worker, process) => (node, worker, process);

let view = ((n, w, p)) => {
  node_id: n |> Int32.of_int,
  worker_id: w |> Int32.of_int,
  process_id: p |> Int32.of_int,
};

let to_string = ((n, w, p)) => Printf.sprintf("<%d,%d,%d>", n, w, p);

let equal = ((n1, w1, p1), (n2, w2, p2)) =>
  n1 == n2 && w1 == w2 && p1 == p2;

let next = ((n, w, p)) => (n, w, p + 1);
