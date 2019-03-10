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

module Make:
  (M: Base) => REGISTRY with type key = M.key and type value = M.value;
