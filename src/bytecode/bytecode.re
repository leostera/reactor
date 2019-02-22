open Model;

type t =
  | Send_message(Pid.t, Message.t): t
  | Spawn(Process.task('state)): t;
