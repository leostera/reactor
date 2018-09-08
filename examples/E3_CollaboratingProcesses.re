open ReActor;
open ReActor_Utils;

module Logger = {
  let __name = "logger";

  type Message.t +=
    | Log(int);

  let logger_f: Process.f(string) =
    (env, prefix) => {
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Log(n) =>
          let diff = Performance.now() - n;
          Js.log({j|$prefix - $n (took $diff ms)|j});
        | _ => ()
        }
      | None => ()
      };
      Become(prefix);
    };

  let logger = spawn(logger_f, "Default =>") |> register(__name);
};

/*
   Clock process:
 */
module Clock = {
  type config = {
    delay: int,
    send_to: Pid.t,
  };
  let clock_f: Process.f(config) =
    (_env, state) => {
      send(state.send_to, Logger.Log(Performance.now()));
      Suspend(state.delay, state);
    };

  let start = spawn(clock_f);
};

switch (whereIs("logger")) {
| Some(pid) =>
  let _ = Clock.start({delay: 0, send_to: pid});
  ();
| None => Js.log("Failed to start logger.")
};
