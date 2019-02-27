type t('value) =
  | Key(Uuidm.t, 'value): t('value);

let create: 'a => t('a);

let __inner_value: t('a) => Uuidm.t;

let equal: (t('a), t('b)) => bool;
