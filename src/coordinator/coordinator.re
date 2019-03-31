let create_pool =
    (~child_count, ~is_child, ~on_child, ~on_parent, ~after_child_spawn) => {
  Array.make(child_count, 0)
  |> Array.iteri((i, _) =>
       switch (is_child()) {
       | false =>
         Logs.info(m =>
           m(
             "[%d] Spawning %d/%d ",
             Platform.Process.pid(),
             i + 1,
             child_count,
           )
         );
         let current =
           switch (Platform.Process.piped_fork()) {
           | `In_child(to_parent, from_parent) =>
             on_child(Platform.Process.pid(), to_parent, from_parent);
             `From_child;
           | `In_parent(pid, write, read) =>
             `From_parent(on_parent(pid, write, read))
           };
         switch (current) {
         | `From_parent(worker) => after_child_spawn(worker)
         | `From_child => ()
         };
       | _ => ()
       }
     );
};

let read_task = Packet.read_from_pipe;
let send_task = (task, fd) =>
  Platform.Process.write(fd, ~buf=Packet.encode(task));

let wait_next_available = (read_fds, write_fds) => {
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
      let cmds = List.map(fd => Packet.read_from_pipe(`Read(fd)), ins);
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
