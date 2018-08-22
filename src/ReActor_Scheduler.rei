type t;

type worker;

let make: (~checkupFreq: int) => t;

let loop: t => unit;
