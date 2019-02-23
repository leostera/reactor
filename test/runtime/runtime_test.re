/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let _ = Scheduler.Policy.default() |> Scheduler.setup;

module System = {
  let spawn = Scheduler.spawn;
  let (<-) = (pid, msg) => Scheduler.send(pid, msg);
};
open System;

/** Application setup */
module Test_actor = {
  type Model.Message.t +=
    | Hola;

  let loop: Model.Process.task(unit) =
    (env, state) => {
      Logs.app(m => m("Looping Test Actor"));
      switch (env.recv()) {
      | Some(Hola) =>
        Logs.app(m => m("Hola!"));
        `Become(state);
      | Some(Model.Message.Debug(str)) =>
        Logs.app(m => m("%s", str));
        `Become(state);
      | _ => `Become(state)
      };
    };

  let start = spawn(loop);
};

let pid = Test_actor.start();
let _ = pid <- Test_actor.Hola;

let pid2 = Test_actor.start();
let _ = pid2 <- Model.Message.Debug("no way josey");

Scheduler.run();
