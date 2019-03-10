let create_pool =
    (~child_count, ~is_child, ~on_child, ~on_parent, ~after_child_spawn) => {
  Array.make(child_count, 0)
  |> Array.iteri((_, _) => {
       let current =
         switch (Platform.Process.piped_fork()) {
         | `In_child(to_parent, from_parent) =>
           on_child(Platform.Process.pid(), to_parent, from_parent);
           `From_child;
         | `In_parent(pid, write, read) =>
           `From_parent(on_parent(pid, write, read))
         };
       switch (is_child(), current) {
       | (false, `From_parent(worker)) => after_child_spawn(worker)
       | _ => ()
       };
     });
};

let send_task = (task, fd) =>
  Platform.Process.write(fd, ~buf=Packet.encode(task));

let wait_next_available = Worker.wait_next_available;

let read_task = Packet.read_from_pipe;
