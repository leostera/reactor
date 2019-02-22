module Policy = Policy;
module Coordinator = Coordinator;

let start = policy => {
  Coordinator.configure(policy);
  Coordinator.start();
};

let send = (pid, msg) => {
  let worker = Coordinator.current() |> Coordinator.worker_for_pid(~pid);
  switch (worker) {
  | None => ()
  | Some(w) => Worker.enqueue_message_for_pid(w, ~pid, ~msg)
  };
};
