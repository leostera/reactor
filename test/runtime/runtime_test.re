/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let _ = Scheduler.Policy.default() |> Scheduler.setup;

module System = {
  let spawn = Scheduler.spawn;
  let (<-) = Scheduler.send;
};
open System;

/** Application setup */
module Test_actor = {
  type message =
    | Hola;

  let loop: Model.Process.task(unit, message) =
    (env, state) => {
      Logs.app(m => m("Looping Test Actor"));
      switch (env.recv()) {
      | Some(Hola) =>
        Logs.app(m => m("Hola!"));
        `Become(state);
      | None => `Become(state)
      };
    };

  let start = spawn(loop);
};

let pid = Test_actor.start();
let _ = pid <- Test_actor.Hola;
let _ = pid <- Test_actor.Hola;
let _ = pid <- Test_actor.Hola;
let _ = pid <- Test_actor.Hola;
let _ = pid <- Test_actor.Hola;
let _ = pid <- Test_actor.Hola;

Scheduler.run();
