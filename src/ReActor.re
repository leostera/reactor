module Runtime = ReActor_Runtime;

module Node = ReActor_Node;

module Scheduler = ReActor_Scheduler;
module Sid = Scheduler.Sid;

module Process = ReActor_Process;
module Pid = Process.Pid;
module Message = Process.Message;

module Tracer = ReActor_Tracer;

/* private */
let __main = ReActor_Node.make();

/* public */
let spawn = (f, args) => ReActor_Node.spawn(__main, f, args);

let exit = ReActor_Node.exit(__main);

let register = ReActor_Node.register(__main);

let whereIs = ReActor_Node.whereIs(__main);

let send = ReActor_Node.send(__main);

let (<-) = send;

let trace = ReActor_Node.trace(__main);
