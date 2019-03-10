open Reactor.System;

let become_reduction = () => {
  Reactor.Node.(Policy.default() |> setup);

  let _ =
    spawn(
      (_, count) =>
        switch (count) {
        | x when x == 0 => exit()
        | _ => `Become(count - 1)
        },
      5,
    );

  Reactor.Node.run();
  Alcotest.(check(bool, "", true, true));
};

let deferred_reduction = () => {
  Reactor.Node.(Policy.default() |> setup);

  let _ =
    spawn(
      (_, count) =>
        switch (count) {
        | x when x == 0 => exit()
        | _ =>
          let promise = Lwt_unix.sleep(0.2) |> Lwt.map(_ => count - 1);
          `Defer(promise);
        },
      5,
    );

  Reactor.Node.run();
  Alcotest.(check(bool, "", true, true));
};

let suite = [
  ("Deferred reductions", `Slow, deferred_reduction),
  ("Become reductions", `Slow, become_reduction),
];

let () = Alcotest.run("Reactor", [("reductions", suite)]);
