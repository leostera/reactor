/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);

open Reactor.System;

module Async_actor = {
  type state = {
    id: string,
    delay: float,
    count: int,
  };

  let loop: Reactor.Process.task(state) =
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        let {id, delay, count} = state;
        Logs.app(m => m("[%s] delay=%.2f count=%d", id, delay, count));
        env.self() <- msg;
        let promise =
          delay
          |> Lwt_unix.sleep
          |> Lwt.map(_ => {...state, count: count + 1});
        `Defer(promise);
      | _ => `Become(state)
      };
    };

  let start = (~delay, ~id) => {
    let pid = spawn(loop, {delay, count: 0, id});
    pid <- id;
    pid;
  };
};

Array.init(
  100,
  index => {
    let id = index |> string_of_int;
    let delay = 0.5 +. float_of_int(index);
    Async_actor.start(~id, ~delay);
  },
)
|> ignore;

Reactor.Scheduler.run();
