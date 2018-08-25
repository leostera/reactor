[@bs.val]
external __unsafe_setTimeout: (unit => 'a, int) => unit = "setTimeout";

let defer = __unsafe_setTimeout;
