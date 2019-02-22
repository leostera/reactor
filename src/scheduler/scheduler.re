module Policy = Policy;
module Coordinator = Coordinator;

let start = policy => {
  Coordinator.configure(policy);
  Coordinator.start();
};

let send = (pid, msg) => {
  Coordinator.current()
  |> Coordinator.handle_task(~task=Bytecode.Send_message(pid, msg));
};
