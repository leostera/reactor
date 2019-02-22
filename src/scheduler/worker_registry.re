module Table =
  Hashtbl.Make({
    type t = Int32.t;
    let equal = Int32.equal;
    let hash = Int32.to_int;
  });

type t = Table.t(Worker.t);

let create = () => Table.create(1024);

let find = Table.find_opt;

let register = (r, key, value) => {
  Table.add(r, key, value);
  r;
};

let unregister = (r, key) => {
  Table.remove(r, key);
  r;
};

let size = Table.length;

let ids = Table.to_seq_keys;

let workers = Table.to_seq_values;
