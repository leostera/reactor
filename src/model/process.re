type behavior('s) = [ | `Become('s) | `Terminate | `Defer(Lwt.t('s))];

type env('s) = {
  self: unit => Pid.t,
  recv: unit => option(string),
};

type task('s) = (env('s), 's) => behavior('s);

type t = {
  pid: Pid.t,
  mailbox: Mailbox.t,
};

let send = (proc, msg) => Mailbox.send(proc.mailbox, msg);

let recv = (proc, ()) => Mailbox.recv(proc.mailbox);

let make = pid => {pid, mailbox: Mailbox.create()};
