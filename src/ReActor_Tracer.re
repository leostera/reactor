open ReActor_Process;

type t = {
  matcher: (Pid.t, Message.t) => bool,
  handler: Message.t => unit,
  timeout: int,
};

let trace: (t, Pid.t, Message.t) => unit =
  (tracer, pid, message) =>
    tracer.matcher(pid, message) ? tracer.handler(message) : ();
