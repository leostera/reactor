open ReActor;
open ReActor_Utils;
open ReActor_Runtime;

/*
  Logger process
 */

type Message.t +=
  | Log(int);

let logger_f: Process.f(string) =
  (env, prefix) => {
    switch (env.recv()) {
    | Some(m) =>
      let now = Date.now();
      switch (m) {
      | Log(n) => Js.log({j|$prefix - $now - $n|j})
      | _ => ()
      };
    | None => ()
    };
    Become(prefix);
  };

let logger = spawn(logger_f, "Default =>") |> register("logger");

Js.log("What!");

defer(() => send(logger, Log(1)), 0);
defer(() => send(logger, Log(2)), 500);
defer(() => send(logger, Log(3)), 1000);
defer(() => send(logger, Log(4)), 3000);
