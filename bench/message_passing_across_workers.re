open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Node.(Policy.default() |> setup);

let msg = Bytes.(create(64998) |> to_string);

let counter =
  spawn(
    (ctx, msg_count) =>
      switch (ctx.recv(), msg_count) {
      | (_, 0) => exit()
      | (Some(_), _) => `Become(msg_count - 1)
      | _ => `Become(msg_count)
      },
    1_000,
  );

let sender =
  spawn(
    (_, receiver) => {
      receiver <- msg;
      `Become(receiver);
    },
    counter,
  );

Reactor.Node.run();
