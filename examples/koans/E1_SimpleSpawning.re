open! ReActor;
open ReActor_Utils;
open ReActor_Runtime;

/**
  Sample state that includes a simple integer counter.
  */
type sample = {n: int};

/**
  A counter process that will increment it's counter by one on each evaluation.
  */
let counter: Process.f(sample) = (_env, state) => Become({n: state.n + 1});

/**
  A counter process that will increment it's counter by one on each evaluation
  and will log out a message when the counter reaches a certain number.
  */
let printing_counter: int => Process.f(sample) =
  (number, env, state) => {
    if (state.n == number) {
      let pid = env.self() |> Pid.toString;
      Js.log({j|$pid :: Reached number $number|j});
    };
    Become({n: state.n + 1});
  };

/**
  A list of 3 counter process' pids.
  */
let counter_pids: list(Pid.t) =
  Array.make(3, 0)
  |> Array.map(i => spawn(counter, {n: i}))
  |> Array.to_list;

/**
  A list of 3 printing_counter process' pids set to print at number 2112.
  */
let counter_pids: list(Pid.t) =
  Array.make(3, 0)
  |> Array.map(i => spawn(printing_counter(2112), {n: i}))
  |> Array.to_list;
