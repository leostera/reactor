open Model;

type t =
  | Send_message(Pid.t('message), 'message): t
  | Spawn(Pid.t('message), Process.task('state, 'message), 'state): t;

let to_string =
  fun
  | Send_message(pid, _) =>
    Printf.sprintf("Send_message(%s)", pid |> Pid.to_string)
  | Spawn(pid, _, _) => Printf.sprintf("Spawn(%s)", pid |> Pid.to_string);
