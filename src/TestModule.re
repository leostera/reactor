open ReActor;
open FFI_Runtime;

type sample = {counter: int};

let rec noop: Process.f(sample) =
  (env, state) => env.loop({counter: state.counter + 1});

let pids = [
  spawn(noop, {counter: 0}),
  spawn(noop, {counter: 0}),
  spawn(noop, {counter: 0}),
  spawn(noop, {counter: 0}),
];
