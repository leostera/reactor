module Policy = Policy;
module Coordinator = Coordinator;

let setup = Coordinator.setup;

let run = Coordinator.Tasks.run;

let send = (pid, msg) => Coordinator.Tasks.send_message(~pid, ~msg);

let spawn = (task, state) => Coordinator.Tasks.spawn(~task, ~state);

let halt = Coordinator.Tasks.halt;
