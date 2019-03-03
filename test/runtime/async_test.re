/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);
/* Scheduler.Policy.custom(~worker_count=1) |> Scheduler.setup; */

open Reactor.System;

module Message = {
  type t = [ | `Iter(string)];

  let encode: t => string = x => Marshal.to_string(x, []);
  let decode: string => t = x => Marshal.from_string(x, 0);

  let iter: (Reactor.Pid.t, string) => unit =
    (pid, text) => pid <- (`Iter(text) |> encode);
};

module Sync_actor = {
  let loop: Reactor.Process.task(int) =
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        let state =
          switch (msg |> Message.decode) {
          | `Iter(text) =>
            /* We have to slow down logs a little bit */
            if (state mod 5000 == 0) {
              Logs.app(m =>
                m("[Sync] id=%s state=%s", text, state |> string_of_int)
              );
            };
            Message.iter(env.self(), text);
            state + 1;
          };
        `Become(state);
      | _ => `Become(state)
      };
    };

  let start = id => {
    let pid = spawn(loop, 0);
    Message.iter(pid, id);
    pid;
  };
};

module Async_actor = {
  let loop = (delay: float): Reactor.Process.task(int) =>
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        let promise =
          switch (msg |> Message.decode) {
          | `Iter(text) =>
            Logs.app(m =>
              m("[Async]: id=%s state=%s", text, state |> string_of_int)
            );
            Message.iter(env.self(), text);
            Lwt_unix.sleep(delay) |> Lwt.map(_ => state + 1);
          };
        `Defer(promise);
      | _ => `Become(state)
      };
    };

  let start = (~delay, ~id) => {
    let pid = spawn(loop(delay), 0);
    Message.iter(pid, id);
    pid;
  };
};

Array.init(
  4,
  index => {
    let id = index |> string_of_int;
    let delay = 1.0 +. float_of_int(index);
    Async_actor.start(~id, ~delay);
  },
)
|> ignore;

/* Array.init(1, index => index |> string_of_int |> Sync_actor.start) |> ignore; */

Reactor.Scheduler.run();