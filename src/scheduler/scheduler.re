module Policy = Policy;
module Coordinator = Coordinator;

let setup = Coordinator.setup;

let run = Coordinator.run;

let send = (pid, msg) => {
  Logs.info(m =>
    m("Sending message from process %i", Platform.Process.pid())
  );
  Coordinator.Tasks.send_message(~pid, ~msg);
};

let spawn = (task, state) => {
  Coordinator.Tasks.spawn(~task, ~state);
};
