Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Debug));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
let _ = Scheduler.Policy.default() |> Scheduler.start;

/** Application setup */
let pid1 = Model.Pid.make(0, 0, 1);
Scheduler.send(pid1, Model.Message.Exit_signal);
