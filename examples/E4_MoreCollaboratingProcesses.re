open ReActor;
open ReActor_Utils;

module Logger = {
  let __name = "logger";

  type Message.t +=
    | Log(int);

  let logger_f: Process.f(string) =
    (env, prefix) =>
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Log(n) =>
          Js.log({j|$prefix$n|j});
          Become(prefix);
        | _ => Become(prefix)
        }
      | None => Become(prefix)
      };

  let logger = spawn(logger_f, "Default => ") |> register(__name);
};

module Differ = {
  type config = {
    send_to: Pid.t,
    wrap: int => Message.t,
  };

  type Message.t +=
    | Diff(int);

  let f: Process.f(config) =
    (env, config) =>
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Diff(t) =>
          let delta = Performance.now() - t;
          send(config.send_to, config.wrap(delta));
          Become(config);
        | _ => Become(config)
        }
      | None => Become(config)
      };

  let start = spawn(f);
};

module Clock = {
  type config = {
    delay: int,
    send_to: Pid.t,
    wrap: int => Message.t,
  };
  let clock_f: Process.f(config) =
    (_env, config) => {
      send(config.send_to, config.wrap(Performance.now()));
      Become(config);
    };

  let start = spawn(clock_f);
};

switch (where_is("logger")) {
| Some(pid) =>
  Array.make(10, 0)
  |> Array.iter(_i => {
       let differ = Differ.start({send_to: pid, wrap: x => Logger.Log(x)});
       let _clocks =
         Clock.start({
           delay: 100,
           send_to: differ,
           wrap: x => Differ.Diff(x),
         });
       ();
     });
  ();
| None => Js.log("Failed to start logger.")
};
