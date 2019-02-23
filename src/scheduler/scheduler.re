module Policy = Policy;
module Coordinator = Coordinator;

let setup = Coordinator.setup;

let run = Coordinator.run;

let send = (pid, msg) => {
  Coordinator.current() |> Coordinator.Tasks.send_message(~pid, ~msg);
};

let spawn = (task, state) => {
  Coordinator.current() |> Coordinator.Tasks.spawn(~task, ~state);
};
