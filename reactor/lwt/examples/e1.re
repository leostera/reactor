open Reactor;

/** Setup loggers */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

module S = Reactor_lwt.Scheduler;
let s = S.make();

type sample = {n: int};

let printing_counter: Process.task(sample) =
  (env, state) => {
    let pid = Process.(env.self()) |> Pid.to_string;
    Logs.debug(m => m("%s :: Reached number %d", pid, state.n));
    `Become({n: state.n + 1});
  };

Array.make(10, 0) |> Array.map(i => S.spawn(s, printing_counter, {n: i}));

S.run(s);
