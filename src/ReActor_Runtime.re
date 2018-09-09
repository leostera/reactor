[@bs.val]
external __unsafe_setImmediate: (unit => 'a) => unit = "setImmediate";
let nextTick = __unsafe_setImmediate;

[@bs.val]
external __unsafe_setTimeout: (unit => 'a, int) => unit = "setTimeout";
let defer = __unsafe_setTimeout;

[@bs.val]
external __unsafe_requestIdleCallback: (unit => 'a) => unit =
  "requestIdleCallback";
let onIdle = __unsafe_requestIdleCallback;

[@bs.val]
external __unsafe_requestAnimationFrame: (unit => 'a) => unit =
  "requestAnimationFrame";
let onAnimationFrame = __unsafe_requestAnimationFrame;

[@bs.val]
external __unsafe_hardwareConcurrency: int =
  "self.navigator.hardwareConcurrency";
let hardwareConcurrency = __unsafe_hardwareConcurrency;
