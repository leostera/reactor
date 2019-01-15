/**
  Example 3: Two collaborating processes sending messages.
  */
open Reactor;

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module S = Reactor_lwt.Scheduler;
module R = Reactor_lwt.Runtime;
let s = S.make();

module Logger = {
  type Message.t +=
    | Log(float);

  let logger_f: Process.task(string) =
    (env, prefix) => {
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Log(n) => Logs.app(m => m("%s %.0f", prefix, n))
        | _ => ()
        }
      | None => ()
      };
      `Become(prefix);
    };

  let start = S.spawn(s, logger_f);
};

module Clock = {
  type config = {send_to: Pid.t};
  let clock_f: Process.task(config) =
    (_env, state) => {
      S.send(s, state.send_to, Logger.Log(Unix.time()));
      `Become(state);
    };

  let start = S.spawn(s, clock_f);
};

let logger = Logger.start("Clock :: ");
let clock = Clock.start(Clock.{send_to: logger});

S.run();
