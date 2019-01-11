module Node = Node;
module Scheduler = Scheduler;
module Process = Process;
module Tracer = Tracer;

/* private */
let __main = Node.make(1);

/* public */
let spawn = (f, args) => Node.spawn(__main, f, args);

let exit = Node.exit(__main);

let register = Node.register(__main);

let whereIs = Node.whereIs(__main);

let send = Node.send(__main);

let (<-) = send;

let trace = Node.trace(__main);
