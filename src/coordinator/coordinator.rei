let create_pool:
  (
    ~child_count: int,
    ~is_child: unit => bool,
    ~on_child: (int, Platform.Process.write_fd, Platform.Process.read_fd) =>
               unit,
    ~on_parent: (
                  [> | `Child_pid(int)],
                  Platform.Process.write_fd,
                  Platform.Process.read_fd
                ) =>
                'a,
    ~after_child_spawn: 'a => unit
  ) =>
  unit;

let send_task: (Bytecode.t, Unix.file_descr) => unit;

let wait_next_available:
  (
    list([ | `Read(Unix.file_descr)]),
    list([ | `Write(Unix.file_descr)])
  ) =>
  (
    [> | `Receive(list(Bytecode.t)) | `Wait],
    [> | `Send(list(Unix.file_descr)) | `Wait],
  );

let read_task: [< | `Read(Unix.file_descr)] => Bytecode.t;
