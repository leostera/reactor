/**
  The type of the tracer.
  */
type t = {
  matcher: (ReActor_Process.Pid.t, ReActor_Process.Message.t) => bool,
  handler: ReActor_Process.Message.t => unit,
  timeout: int,
};

let trace: (t, ReActor_Process.Pid.t, ReActor_Process.Message.t) => unit;
