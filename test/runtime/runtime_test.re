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
  module Message = {
    type t = [ | `Hola];

    let encode =
      fun
      | `Hola => "Hola";

    let decode =
      fun
      | "Hola" => Some(`Hola)
      | _ => None;

    let say_hi = pid => pid <- (`Hola |> encode);
  };

  let loop: Model.Process.task(unit) =
    (env, state) => {
      Logs.app(m => m("Looping Test Actor"));
      switch (env.recv()) {
      | Some(msg) =>
        switch (msg |> Message.decode) {
        | Some(`Hola) => Logs.app(m => m("Hola!"))
        | _ => ()
        };
        `Become(state);
      | _ => `Become(state)
      };
    };

  let start = spawn(loop);
};

let pid = Test_actor.start();
let _ = Test_actor.Message.say_hi(pid);

Scheduler.run();
