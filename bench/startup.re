open Reactor.System;

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);

let _ = spawn((_, _) => exit(), ());

Reactor.Scheduler.run();
