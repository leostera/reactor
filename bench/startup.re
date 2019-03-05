open Reactor.System;

Fmt_tty.setup_std_outputs();
// Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** Logging setup */
let result =
  Benchmark.latencyN(
    ~repeat=10,
    10L,
    [
      (
        "Startup",
        () => {
          Reactor.Scheduler.(Policy.default() |> setup);
          let _ = spawn((_, _) => exit(), ());
          Reactor.Scheduler.run();
        },
        (),
      ),
    ],
  );

Logs.app(m => m("[%d] Startup cost", Unix.getpid()));
Benchmark.tabulate(result);
