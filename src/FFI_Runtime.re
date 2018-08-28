[@bs.val]
external __unsafe_setImmediate: (unit => 'a, int) => unit = "setImmediate";

let defer = __unsafe_setImmediate;

let nextTick = f => defer(f, 0);
