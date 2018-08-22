type timeout_t;
[@bs.val]
external __unsafe_setTimeout: (unit => 'a, int) => timeout_t = "setTimeout";

let defer = __unsafe_setTimeout;
