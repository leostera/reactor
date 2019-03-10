open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let begin_at = Unix.time();
Reactor.Node.(Policy.default() |> setup);

spawn(
  (_, (last, this, count)) =>
    switch (count) {
    | 1_000_000 =>
      Logs.app(m => m("%d", this));
      exit();
    | _ => `Become((this, last + this, count + 1))
    },
  (0, 1, 0),
)
|> ignore;

Reactor.Node.run();

let end_at = Unix.time();
let delta = end_at -. begin_at;
Logs.app(m => m("Computed 1,000,000th Fibonacci number in: %.2fs", delta));
