/**
  Example 2: Long-running process that receives messages.
  */
open Reactor;

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module S = Reactor_lwt.Scheduler;
module R = Reactor_lwt.Runtime;
let s = S.make();

type Message.t +=
  | Log(int);

let logger_f: Process.task(string) =
  (env, prefix) => {
    switch (env.recv()) {
    | Some(m) =>
      let now = Unix.time();
      switch (m) {
      | Log(n) => Logs.app(m => m("%s %d at %.0f", prefix, n, now))
      | _ => ()
      };
    | None => ()
    };
    `Become(prefix);
  };

let logger = S.spawn(s, logger_f, "Default =>");

R.defer(() => S.send(s, logger, Log(1)));
R.defer(() => S.send(s, logger, Log(2)));
R.defer(() => S.send(s, logger, Log(3)));
R.defer(() => S.send(s, logger, Log(4)));

S.run();
