open Model;

type t =
  | Send_message(Pid.t, Message.t): t
  | Spawn(Pid.t, Process.task('state), 'state): t;

let to_string =
  fun
  | Send_message(pid, _) =>
    Printf.sprintf("Send_message(%s)", pid |> Model.Pid.to_string)
  | Spawn(pid, _, _) =>
    Printf.sprintf("Spawn(%s)", pid |> Model.Pid.to_string);
