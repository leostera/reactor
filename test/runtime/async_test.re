/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);

open Reactor.System;

module Async_actor = {
  type state = {
    delay: float,
    count: int,
  };

  let loop: Reactor.Process.task(state) =
    (env, state) => {
      switch (state.count) {
      | x when x > 100 => exit()
      | _ =>
        let pid = env.self() |> Reactor.Pid.to_string;
        let {delay, count} = state;
        Logs.app(m => m("%s @@ delay=%.2f count=%d", pid, delay, count));
        let promise =
          delay
          |> Lwt_unix.sleep
          |> Lwt.map(_ => {...state, count: count + 1});
        `Defer(promise);
      };
    };

  let start = (~delay) => spawn(loop, {delay, count: 0});
};

Array.init(
  10,
  index => {
    let delay = 0.01 +. float_of_int(index);
    Async_actor.start(~delay);
  },
)
|> ignore;

Reactor.Scheduler.run();
