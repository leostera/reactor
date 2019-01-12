type behavior('s) = [ | `Become('s) | `Terminate];

type env('s) = {self: unit => Pid.t};

type task('s) = (env('s), 's) => behavior('s);

type t = {
  pid: Pid.t,
  mailbox: ref(list(Message.t)),
};

let send = (msg, process) => {
  process.mailbox := [msg, ...process.mailbox^];
};

let make = pid => {pid, mailbox: ref([])};
