module Pid = Pid;
module Message = Message;

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

module Registry: Registry.REGISTRY with type key = Pid.t and type value = t =
  Registry.Make({
    type key = Pid.t;
    type value = t;
    let equal = Pid.equal;
    let hash = Pid.hash;
  });
