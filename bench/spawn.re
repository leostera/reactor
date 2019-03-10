open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let begin_at = Unix.time();
Reactor.Node.(Policy.default() |> setup);

let spawn_counter = pid =>
  spawn((_ctx, _) => {
    pid <- "add";
    `Terminate;
  });

let _ =
  spawn(
    (ctx, (proc_count, i)) =>
      switch (ctx.recv(), i + 1) {
      | (_, 0) =>
        Array.make(proc_count, 0)
        |> Array.map(spawn_counter(ctx.self()))
        |> ignore;
        Logs.app(m => m("%d actors spawned", proc_count));
        `Become((proc_count, 0));
      | (Some("add"), i') =>
        switch (i' == proc_count) {
        | true =>
          Logs.app(m => m("%d actors finished.", proc_count));
          exit();
        | _ => `Become((proc_count, i'))
        }
      | (_, _) => `Become((proc_count, i))
      },
    (1_000_000, (-1)),
  );

Reactor.Node.run();

let end_at = Unix.time();
let delta = end_at -. begin_at;
Logs.app(m => m("Spawned and terminated 1,000,000 Actors in: %.2fs", delta));
