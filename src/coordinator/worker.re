module Bytecode = Bytecode;

let wait_next_available:
  (
    list([ | `Read(Unix.file_descr)]),
    list([ | `Write(Unix.file_descr)])
  ) =>
  (
    [> | `Receive(list(Bytecode.t)) | `Wait],
    [> | `Send(list(Unix.file_descr)) | `Wait],
  ) =
  (read_fds, write_fds) => {
    let read_fds = List.map((`Read(fd)) => fd, read_fds);

    let write_fds = List.map((`Write(fd)) => fd, write_fds);

    let (reads, writes, _errors) =
      Platform.Process.select(
        ~read=read_fds,
        ~write=write_fds,
        ~except=[],
        ~timeout=-1.0,
      );

    let receive =
      switch (reads) {
      | `Read(ins) when List.length(ins) > 0 =>
        let cmds: list(Bytecode.t) =
          List.map(fd => Packet.read_from_pipe(`Read(fd)), ins);

        `Receive(cmds);
      | _ => `Wait
      };

    let sends =
      switch (writes) {
      | `Write(outs) when List.length(outs) > 0 => `Send(outs)
      | _ => `Wait
      };

    (receive, sends);
  };
