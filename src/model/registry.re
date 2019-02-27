module Pid = Process.Pid;
module Process = Process.Process;

type eq('a, 'b) =
  | Type_eq: eq('a, 'a);

let eq:
  type a b.
    (Registry_key.t(a), Registry_key.t(b)) =>
    [> | `Equal(eq(a, b)) | `Not_equal] =
  (k, k') => Registry_key.equal(k, k') ? `Equal(Type_eq) : `Not_equal;

type binding =
  | B(Registry_key.t('v), 'v): binding;

module Table =
  Hashtbl.Make({
    type t = Uuidm.t;
    let equal = Uuidm.equal;
    /* TODO(@ostera): Use a proper hash function */
    let hash = Hashtbl.hash;
  });

type t = Table.t(binding);

let create = () => Table.create(1024);

let find_value: type a. (Registry_key.t(a), binding) => option(a) =
  (k, b) => {
    switch (b) {
    | [@implicit_arity] B(k', v) =>
      switch (eq(k, k')) {
      | `Equal(Type_eq) => Some(v)
      | _ => None
      }
    };
  };

let find = (r, pid) => {
  let key = Pid.unique_key(pid);
  let inner_key = Registry_key.__inner_value(key);
  let value = Table.find_opt(r, inner_key);

  switch (value) {
  | Some(b) => find_value(b, key)
  | None => None
  };
};

let register = (r, pid, proc) =>
  Table.add(r, Pid.unique_key(pid) |> Registry_key.__inner_value, proc);

let unregister = (r, pid) =>
  Table.remove(r, Pid.unique_key(pid) |> Registry_key.__inner_value);

let length = Table.length;
