type t = {
  last_pid: ref(option(Pid.t)),
  process_registry: ref(Registry.t),
};

module type BASE = {
  let defer: (unit => unit) => unit;
  let run: unit => unit;
};

module type SCHEDULER = {
  let make: unit => t;
  let spawn: (t, Process.task('state), 'state) => Pid.t;
  let run: unit => unit;
};

module Make = (M: BASE) : SCHEDULER => {
  let make = () => {
    last_pid: ref(None),
    process_registry: ref(Registry.create()),
  };

  let spawn = (scheduler, work, initial_state) => {
    let pid =
      switch (scheduler.last_pid^) {
      | None => Pid.make(0)
      | Some(pid) => pid |> Pid.next
      };
    let proc = pid |> Process.make;
    let env = Process.{self: () => pid};

    let unreg = () =>
      scheduler.process_registry :=
        Registry.unregister(scheduler.process_registry^, pid);

    let rec run = args =>
      switch (work(env, args)) {
      | exception ex =>
        Logs.debug(m =>
          m(
            "Process Terminated: %s threw %s",
            pid |> Pid.to_string,
            ex |> Printexc.to_string,
          )
        );
        unreg();
      | `Terminate => unreg()
      | `Become(next_state) => M.defer(() => run(next_state))
      };
    run(initial_state);

    scheduler.last_pid := Some(pid);
    scheduler.process_registry :=
      Registry.register(scheduler.process_registry^, pid, proc);

    pid;
  };

  let run = M.run;
};
