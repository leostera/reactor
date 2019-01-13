type behavior('s) = [ | `Become('s) | `Terminate];

type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(Message.t),
};

type task('s) = (env('s), 's) => behavior('s);

type t = {
  pid: Pid.t,
  mailbox: Queue.t(Message.t),
};

let send = (proc, msg) => {
  Queue.push(msg, proc.mailbox);
};

let recv = (proc, ()) => {
  switch (Queue.pop(proc.mailbox)) {
  | exception Queue.Empty => None
  | msg => Some(msg)
  };
};

let make = pid => {pid, mailbox: Queue.create()};
