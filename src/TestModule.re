open ReActor;
open FFI_Runtime;

type sample = {counter: int};

let rec noop: Process.f(sample) =
  (env, state) => env.loop({counter: state.counter + 1});

let pids =
  Array.make(1000000, 0)
  |> Array.to_list
  |> List.map(i => spawn(noop, {counter: i}));
