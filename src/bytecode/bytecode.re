module Process = Reactor_process.Process;

type t =
  | Halt
  | Send_message(Process.Pid.t, Process.Message.t): t
  | Spawn(Process.Pid.t, Process.task('state), 'state): t;

let to_string =
  fun
  | Halt => Printf.sprintf("Halt")
  | Send_message(pid, _) =>
    Printf.sprintf("Send_message(%s)", pid |> Process.Pid.to_string)
  | Spawn(pid, _, _) =>
    Printf.sprintf("Spawn(%s)", pid |> Process.Pid.to_string);
