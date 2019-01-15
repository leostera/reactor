type callback('a) = unit => 'a;
let nextTick: callback('a) => unit;
let defer: (unit => 'a, int) => unit;
let onIdle: callback('a) => unit;
let onAnimationFrame: callback('a) => unit;
let hardwareConcurrency: int;
