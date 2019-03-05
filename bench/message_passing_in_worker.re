open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);

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

Reactor.Scheduler.run();
