module Node = ReActor_Node;
module Process = ReActor_Process;
module Pid = ReActor_Process.Pid;
module Message = ReActor_Process.Message;
module Tracer = ReActor_Tracer;

let __main = ReActor_Node.make();

let spawn = (f, args) => ReActor_Node.spawn(__main, f, args);

let exit = ReActor_Node.exit(__main);

let register = ReActor_Node.register(__main);

let where_is = ReActor_Node.where_is(__main);

let send = ReActor_Node.send(__main);

let trace = ReActor_Node.trace(__main);
