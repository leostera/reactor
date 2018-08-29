[@bs.val]
external __unsafe_setImmediate: (unit => 'a) => unit = "setImmediate";

[@bs.val]
external __unsafe_setTimeout: (unit => 'a, int) => unit = "setTimeout";

let defer = __unsafe_setTimeout;

let nextTick = __unsafe_setImmediate;
