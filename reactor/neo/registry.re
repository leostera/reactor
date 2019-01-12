module Table =
  Hashtbl.Make({
    type t = Pid.t;
    let equal = Pid.equal;
    let hash = Hashtbl.hash;
  });

type t = Table.t(Process.t);

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
