type t('m);

let send: (t('m), 'm) => unit;

let recv: t('m) => option('m);

let create: unit => t('m);
