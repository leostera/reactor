type t('a);

let create: unit => t('a);

let queue: (t('a), 'a) => t('a);

let next: t('a) => option('a);

let to_seq: t('a) => Seq.t('a);
