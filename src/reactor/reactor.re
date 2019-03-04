module Pid = Model.Pid;
module Process = Model.Process;
module Scheduler = Scheduler;

module System = {
  let exit = () => {
    Scheduler.halt();
    `Terminate;
  };
  let spawn = Scheduler.spawn;
  let (<-) = (pid, msg) => Scheduler.send(pid, msg);
};
