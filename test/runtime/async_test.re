/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Error));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);
/* Scheduler.Policy.custom(~worker_count=1) |> Scheduler.setup; */

open Reactor.System;

/** Application setup */
module Test_actor = {
  module Message = {
    type t = [ | `Iter(string)];

    let encode: t => string = x => Marshal.to_string(x, []);
    let decode: string => t = x => Marshal.from_string(x, 0);

    let loop: (Reactor.Pid.t, string) => unit =
      (pid, text) => pid <- (`Iter(text) |> encode);
  };

  let loop: Reactor.Process.task(int) =
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        let state =
          switch (msg |> Message.decode) {
          | `Iter(text) =>
            Logs.app(m => m("Sync %s: %s", text, state |> string_of_int));
            Message.loop(env.self(), text);
            state + 1;
          };
        `Become(state);
      | _ => `Become(state)
      };
    };

  let start = id => {
    let pid = spawn(loop, 0);
    Message.loop(pid, id);
    ();
  };
};

/** Application setup */
module Async_actor = {
  module Message = {
    type t = [ | `Iter(string)];

    let encode: t => string = x => Marshal.to_string(x, []);
    let decode: string => t = x => Marshal.from_string(x, 0);

    let loop: (Reactor.Pid.t, string) => unit =
      (pid, text) => pid <- (`Iter(text) |> encode);
  };

  let loop: Reactor.Process.task(int) =
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        let promise =
          switch (msg |> Message.decode) {
          | `Iter(text) =>
            Logs.app(m => m("Async %s: %s", text, state |> string_of_int));
            Message.loop(env.self(), text);
            Lwt_io.(read_line(stdin)) |> Lwt.map(_ => state + 1);
          };
        `Defer(promise);
      | _ => `Become(state)
      };
    };

  let start = id => {
    let pid = spawn(loop, 0);
    Message.loop(pid, id);
    ();
  };
};

Array.init(4, id => id |> string_of_int |> Test_actor.start) |> ignore;
Array.init(1, id => id |> string_of_int |> Async_actor.start) |> ignore;

Reactor.Scheduler.run();