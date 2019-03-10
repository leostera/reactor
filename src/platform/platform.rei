/**
  Abstraction layer on top of the current platform for a little more
  type-safety on communicating processes.
  */
module Process: {
  type read_fd = [ | `Read(Unix.file_descr)];

  type write_fd = [ | `Write(Unix.file_descr)];

  /**
    [pid()] evaluates to the current operating-system process identifier number.
    */
  let pid: unit => int;

  /**
    [kill(n)]
    */
  let kill: int => unit;

  /**
    [write(fd, ~buf=bytes)] will write [bytes] into [fd].
    */
  let write: (Unix.file_descr, ~buf: bytes) => unit;

  /**
    [read(fd, ~len=n)] will read [n] bytes out of [fd].
    */
  let read: (Unix.file_descr, ~len: int) => bytes;

  let pipe: unit => (read_fd, write_fd);

  let piped_fork:
    unit =>
    [>
      | `In_child(write_fd, read_fd)
      | `In_parent([> | `Child_pid(int)], write_fd, read_fd)
    ];

  let select:
    (
      ~read: list(Unix.file_descr),
      ~write: list(Unix.file_descr),
      ~except: list(Unix.file_descr),
      ~timeout: float
    ) =>
    (
      [> | `Read(list(Unix.file_descr))],
      [> | `Write(list(Unix.file_descr))],
      [> | `Except(list(Unix.file_descr))],
    );
};

/** [cpu_count()] evaluates to the current CPU count. */
let cpu_count: unit => int;
