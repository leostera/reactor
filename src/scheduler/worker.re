module Unix = UnixLabels;

type t = {
  id: int,
  unix_pid: int,
  pipe_to_worker: Unix.file_descr,
  pipe_from_worker: Unix.file_descr,
};

let id = t => t.id;

let wait_next_available = workers => {
  let read_fds =
    workers |> Seq.fold_left((fds, w) => [w.pipe_from_worker, ...fds], []);

  switch (
    Platform.Process.select(
      ~read=read_fds,
      ~write=[],
      ~except=[],
      ~timeout=-1.0,
    )
  ) {
  | (`Read(ins), _, _) when List.length(ins) > 0 =>
    let cmds =
      ins
      |> List.to_seq
      |> Seq.map(fd => {
           let marshalled_instruction_size = 57;
           let raw =
             Platform.Process.read(fd, ~len=marshalled_instruction_size);
           let cmd: Bytecode.t = Marshal.from_bytes(raw, 0);
           cmd;
         });

    `Receive(cmds);
  | _ => `Wait
  /*
   | (_, `Write(outs), _) when List.length(outs) > 0 =>
     let cmd = Task_queue.next(tasks);
     let raw = Marshal.to_bytes(cmd, [Marshal.Closures]);
     outs |> List.iter(Platform.Process.write(~buf=raw));
     do_loop();
     */
  };
};

let start = () => {
  switch (Platform.Process.piped_fork()) {
  | `In_child(`Write(to_parent), `Read(_from_parent)) =>
    /* On Worker */
    let pid = Platform.Process.pid();
    let task =
      Bytecode.Send_message(
        Model.Pid.make(0, 0, 0),
        Model.Message.Debug("what"),
      );
    let buf = Marshal.to_bytes(task, []);
    let write = Platform.Process.write(to_parent);
    let rec loopy = () => {
      write(~buf);
      Unix.sleep(2);
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
