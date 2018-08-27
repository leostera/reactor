[@bs.val]
external __unsafe_setTimeout: (unit => 'a, int) => unit = "setTimeout";

let defer = __unsafe_setTimeout;

let nextTick = f => defer(f, 0);
