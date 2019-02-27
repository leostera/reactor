/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
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
    type t = [ | `Forward(Model.Pid.t, string) | `Print(string)];

    let encode: t => string = x => Marshal.to_string(x, []);
    let decode: string => t = x => Marshal.from_string(x, 0);

    let print: (Model.Pid.t, string) => unit =
      (pid, text) => pid <- (`Print(text) |> encode);

    let fwd: (Model.Pid.t, ~target: Model.Pid.t, string) => unit =
      (pid, ~target, text) => pid <- (`Forward((target, text)) |> encode);
  };

  let loop: Model.Process.task(unit) =
    (env, state) => {
      switch (env.recv()) {
      | Some(msg) =>
        switch (msg |> Message.decode) {
        | `Forward(target, text) => Message.print(target, text)
        | `Print(text) => Logs.app(m => m("[`Print(%s)]", text))
        };
        `Become(state);
      | _ => `Become(state)
      };
    };

  let start = spawn(loop);
};

let pid_1 = Test_actor.start();
let pid_2 = Test_actor.start();

Test_actor.Message.print(pid_1, "hello reactor!");
Test_actor.Message.fwd(pid_2, ~target=pid_1, "forwarded message!");

Scheduler.run();
