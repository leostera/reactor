open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let begin_at = Unix.time();
Reactor.Node.(Policy.default() |> setup);

let msg = Bytes.(create(64998) |> to_string);

let pid =
  spawn(
    (ctx, msg_count) =>
      switch (ctx.recv(), msg_count) {
      | (_, 0) => exit()
      | (Some(msg), _) =>
        ctx.self() <- msg;
        `Become(msg_count - 1);
      | _ => `Become(msg_count)
      },
    100_000,
  );

pid <- msg;

Reactor.Node.run();

let end_at = Unix.time();
let delta = end_at -. begin_at;
Logs.app(m => m("Send 64998 bytes x 100,000 in a worker in: %.2fs", delta));
