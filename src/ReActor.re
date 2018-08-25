module Process = ReActor_Process;

let __main = ReActor_Node.make();

Js.log("Created main scheduler...");
Js.log(__main);

let spawn = (f, args) => ReActor_Node.spawn(__main, f, args);
