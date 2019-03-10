module type Base = {
  type key;

  let equal: (key, key) => bool;

  let hash: key => int;

  type value;
};

module type REGISTRY = {
  type key;

  type value;

  type t;

  let create: (~size: int=?, unit) => t;

  let find: (t, key) => option(value);

  let register: (t, key, value) => unit;

  let unregister: (t, key) => unit;

  let update: (t, key, value) => unit;

  let size: t => int;

  let keys: t => Seq.t(key);

  let values: t => Seq.t(value);
};

module Make =
       (M: Base)
       : (REGISTRY with type key = M.key and type value = M.value) => {
  module Table =
    Hashtbl.Make({
      type t = M.key;
      let equal = M.equal;
      let hash = M.hash;
    });

  type key = M.key;

  type value = M.value;

  type t = Table.t(M.value);

  let create = (~size=1024, ()) => Table.create(size);

  let find = Table.find_opt;

  let register = Table.add;

  let unregister = Table.remove;

  let update = (r, key, value) => {
    unregister(r, key);
    register(r, key, value);
  };

  let size = Table.length;

  let keys = Table.to_seq_keys;

  let values = Table.to_seq_values;
};
