/**
  The type of the tracer
  */
type t = {
  matcher: (Process.Pid.t, Process.Message.t) => bool,
  handler: Process.Message.t => unit,
  timeout: int,
};

let trace: (t, Process.Pid.t, Process.Message.t) => unit;
