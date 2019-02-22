module Unix = UnixLabels;

type t = {
  id: int,
  unix_pid: int,
  pipe_to_worker: Unix.file_descr,
  pipe_from_worker: Unix.file_descr,
};

let id = t => t.id;

let pipes = t => (
  `To_worker(t.pipe_to_worker),
  `From_worker(t.pipe_from_worker),
);

let start = () => {
  switch (Platform.Process.piped_fork()) {
  | `In_child(`Write(to_parent), `Read(_from_parent)) =>
    /* On Worker */
    let pid = Platform.Process.pid();
    let str = Printf.sprintf("hello from %i!", pid);
    let buf = Marshal.to_bytes(str, []);
    let rec loopy = () => {
      Platform.Process.write(~fd=to_parent, ~buf);
      loopy();
      ();
    };
    switch (loopy()) {
    | exception e =>
      let err = Printexc.to_string(e);
      Logs.err(m => m("Uncaught exception in worker (pid %i): %s", pid, err));
      exit(1);
    | _ => None
    };

  | `In_parent(`Child_pid(pid), `Write(to_worker), `Read(from_worker)) =>
    Some({
      id: pid,
      unix_pid: pid,
      pipe_to_worker: to_worker,
      pipe_from_worker: from_worker,
    })
  };
};

let enqueue_message_for_pid = (_worker, ~pid as _, ~msg as _) => ();
