open! ReActor;
open ReActor_Utils;
open ReActor_Runtime;

type sample = {counter: int};

let noop: Process.f(sample) =
  (env, state) => {
    if (state.counter == 2112) {
      let p = env.self() |> Pid.toString;
      Js.log({j|$p got to 2112|j});
    };
    Become({counter: state.counter + 1});
  };

let loop_noop: Process.f(sample) =
  (env, state) => {
    let now = Date.now();
    let counter = state.counter;
    let pid = Pid.toString(env.self());
    Js.log({j|$now - $pid - $counter|j});
    Suspend(100, {counter: state.counter - 1});
  };

let named_slower_pid =
  spawn(loop_noop, {counter: 0}) |> register("slower_pid");
let anon_slower_pid = spawn(loop_noop, {counter: 0});

let pids: list(Pid.t) =
  Array.make(10000, 0)
  |> Array.map(i => spawn(noop, {counter: i}))
  |> Array.to_list;

defer(
  () => {
    let p = anon_slower_pid |> Pid.toString;
    Js.log({j|Anon Killing $p by pid|j});
    exit(anon_slower_pid);
  },
  1500,
);
defer(
  () =>
    switch (whereIs("slower_pid")) {
    | None => Js.log("Could not find pid \"slower_pid\"")
    | Some(pid) =>
      let p = pid |> Pid.toString;
      Js.log({j|Killing $p by name "slower_pid"|j});
      exit(pid);
    },
  3000,
);
