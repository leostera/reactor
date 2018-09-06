open ReActor;
open ReActor_Utils;
open FFI_Runtime;

module DOMLogger = {
  let __name = "logger";

  type state = {node: DOM.node};

  type Message.t +=
    | Log(string);

  let logger_f: Process.f(state) =
    (env, state) =>
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Log(msg) =>
          DOM.withInnerText(state.node, msg);
          Become(state);
        | _ => Become(state)
        }
      | None => Become(state)
      };

  let dom_logger =
    spawn(logger_f, {node: DOM.getElementById("sample")})
    |> register(__name);

  let logInt: int => Message.t =
    s => Log(string_of_int(s) ++ "ms - " ++ Random.shortId());
};

module Differ = {
  type config = {
    send_to: Pid.t,
    wrap: int => Message.t,
  };

  type diff = {
    pid: Pid.t,
    current_time: int,
    random_id: string,
  };

  type Message.t +=
    | Diff(diff);

  let f: Process.f(config) =
    (env, config) =>
      switch (env.recv()) {
      | Some(m) =>
        switch (m) {
        | Diff(t) =>
          let delta = Performance.now() - t.current_time;
          let pid = t.pid |> Pid.toString;
          let self = env.self() |> Pid.toString;
          Js.log({j|Received message from: $pid while running on $self|j});
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
    wrap: (Pid.t, int) => Message.t,
  };
  let clock_f: Process.f(config) =
    (env, config) => {
      send(config.send_to, config.wrap(env.self(), Performance.now()));
      Suspend(config.delay, config);
    };

  let start = spawn(clock_f);
};

trace({
  matcher: (_pid, message) =>
    switch (message) {
    | Differ.Diff(_) => true
    | _ => false
    },
  timeout: 50,
  handler:
    fun
    | Differ.Diff(n) => Js.log({j|Differ got message => $n|j})
    | _ => (),
});

switch (where_is("logger")) {
| Some(pid) =>
  let differ = Differ.start({send_to: pid, wrap: DOMLogger.logInt});
  let _clock =
    Clock.start({
      delay: 0,
      send_to: differ,
      wrap: (pid, x) =>
        Differ.Diff({pid, current_time: x, random_id: Random.shortId()}),
    });
  ();
| None => Js.log("Failed to start logger.")
};
