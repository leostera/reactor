open ReActor;
open ReActor_Utils;
open FFI_Runtime;

module ReactLogger = {
  let __name = "logger";

  type state = {elementId: string};

  type Message.t +=
    | Log(string);

  let component = ReasonReact.statelessComponent("Log");

  let makeComponent = (~text, _children) => {
    ...component,
    render: _self =>
      ReasonReact.createDomElement(
        "div",
        ~props={"name": "logger"},
        [|ReasonReact.string(text)|],
      ),
  };

  let logger_f: Process.f(state) =
    (env, state) => {
      env.recv(
        fun
        | Log(text) => {
            let el = makeComponent(~text, [||]) |> ReasonReact.element;
            ReactDOMRe.renderToElementWithId(el, state.elementId);
            env.loop(state);
          }
        | _ => env.loop(state),
      );
      state;
    };

  let dom_logger =
    spawn(logger_f, {elementId: "sample"}) |> register(__name);

  let logInt: int => Message.t = s => Log(string_of_int(s) ++ "ms");
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
  let differ = Differ.start({send_to: pid, wrap: ReactLogger.logInt});
  let _clock =
    Clock.start({delay: 100, send_to: differ, wrap: x => Differ.Diff(x)});
  ();
| None => Js.log("Failed to start logger.")
};
