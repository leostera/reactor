type t('value) =
  | Key(Uuidm.t, 'value): t('value);

let create = message => Key(Uuidm.v(`V4), message);

let __inner_value = (Key(v, _)) => v;

let equal = (Key(v, _), Key(v', _)) => Uuidm.equal(v, v');
