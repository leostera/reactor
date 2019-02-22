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
  let (worker_out, worker_in) = Unix.pipe();
  let (coord_out, coord_in) = Unix.pipe();

  switch (Unix.fork()) {
  | 0 =>
    /* On Worker */
    let pid = Unix.getpid();
    try (
      {
        Unix.close(coord_in);
        Unix.close(worker_out);
        /* Prepare worker initial state */
        /* Setup read/write loop by reading from coord_out and writing to worker_in */
        let rec loopy = () => {
          let str = Printf.sprintf("hello from %i!", pid);
          let len = str |> String.length;
          let oc = Unix.out_channel_of_descr(worker_in);
          Marshal.to_channel(oc, str, [Marshal.Closures]);
          Logs.app(m => m("[%i] Writing %d bytes", pid, len));
          Unix.sleep(5);
          loopy();
          ();
        };
        loopy();
        None;
      }
    ) {
    | e =>
      let err = Printexc.to_string(e);
      Logs.err(m => m("Uncaught exception in worker (pid %i): %s", pid, err));
      exit(1);
    };
  | child_pid =>
    /* On Coordinator */
    Unix.close(worker_in);
    Unix.close(coord_out);
    Some({
      id: child_pid,
      unix_pid: child_pid,
      pipe_to_worker: coord_in,
      pipe_from_worker: worker_out,
    });
  };
};

let enqueue_message_for_pid = (_worker, ~pid as _, ~msg as _) => ();
