open ReActor;
open ReActor_Utils;
open FFI_Runtime;

module Logger = {
  let __name = "logger";

  type Message.t +=
    | Log(int);

  let logger_f: Process.f(string) =
    (env, prefix) => {
      env.recv(
        fun
        | Log(n) => {
            Js.log({j|$prefix$n|j});
            env.loop(prefix);
          }
        | _ => env.loop(prefix),
      );
      prefix;
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
    (env, config) => {
      env.recv(
        fun
        | Diff(t) => {
            let delta = Performance.now() - t;
            send(config.send_to, config.wrap(delta));
            env.loop(config);
          }
        | _ => env.loop(config),
      );
      config;
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
    (env, config) => {
      env.sleep(
        config.delay,
        () => {
          send(config.send_to, config.wrap(Performance.now()));
          env.loop(config);
        },
      );
      config;
    };

  let start = spawn(clock_f);
};

switch (where_is("logger")) {
| Some(pid) =>
  let differ = Differ.start({send_to: pid, wrap: x => Logger.Log(x)});
  let _clock =
    Clock.start({delay: 0, send_to: differ, wrap: x => Differ.Diff(x)});
  ();
| None => Js.log("Failed to start logger.")
};
