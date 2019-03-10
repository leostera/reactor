type t = (int, int, int);

type view = {
  node_id: int32,
  scheduler_id: int32,
  process_id: int32,
};

let make = (node, scheduler, process) => (node, scheduler, process);

let view = ((n, s, p)) => {
  node_id: n |> Int32.of_int,
  scheduler_id: s |> Int32.of_int,
  process_id: p |> Int32.of_int,
};

let to_string = ((n, s, p)) => Printf.sprintf("<%d,%d,%d>", n, s, p);

let equal = ((n1, s1, p1), (n2, s2, p2)) =>
  n1 == n2 && s1 == s2 && p1 == p2;

let next = ((n, s, p)) => (n, s, p + 1);

/* TODO(@ostera): Replace! */
let hash = Hashtbl.hash;
