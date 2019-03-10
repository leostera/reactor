type t('a);

let create: unit => t('a);

let queue: (t('a), 'a) => unit;

let next: t('a) => option('a);

let to_seq: t('a) => Seq.t('a);

let length: t('a) => int;

let clear: t('a) => unit;
