open Process;

type t = {
  matcher: (Pid.t, Message.t) => bool,
  handler: Message.t => unit,
  timeout: int,
};

let trace = (tracer, pid, message) =>
  tracer.matcher(pid, message) ? tracer.handler(message) : ();
