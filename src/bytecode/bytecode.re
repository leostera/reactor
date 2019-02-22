open Model;

type t =
  | Send_message(Pid.t, Message.t): t
  | Spawn(Process.task('state)): t;

let to_string =
  fun
  | Send_message(pid, _) =>
    Printf.sprintf("Send_message(%s)", pid |> Model.Pid.to_string)
  | Spawn(_) => "Spawn";
