open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

Reactor.Node.(Policy.default() |> setup);
spawn((_, _) => exit(), ());
Reactor.Node.run();

/** Crashes are more frequent when running the node more than once */
Reactor.Node.(Policy.default() |> setup);
spawn((_, _) => exit(), ());
Reactor.Node.run();
