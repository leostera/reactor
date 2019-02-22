module Unix = UnixLabels;

let pipe = () => {
  let (r, w) = Unix.pipe();
  (`Read(r), `Write(w));
};

let write = (fd, ~buf) => {
  let len = buf |> Bytes.length;
  let _ = Unix.write(fd, ~buf, ~pos=0, ~len);
  ();
};

let read = (fd, ~len) => {
  let buf = Bytes.create(256);
  let rec read_pipe = (pos, len) => {
    switch (Unix.read(fd, ~buf, ~pos, ~len)) {
    | exception _ => buf
    | 0 => buf
    | n =>
      switch (n + (buf |> Bytes.length) >= len) {
      | true => buf
      | _ => read_pipe(pos + n, len - n)
      }
    };
  };
  read_pipe(0, len);
};

let fork = () => {
  switch (Unix.fork()) {
  | 0 => `In_child
  | pid => `In_parent(pid)
  };
};

let pid = Unix.getpid;

let piped_fork = () => {
  let (`Read(read_child), `Write(write_child)) = pipe();
  let (`Read(read_parent), `Write(write_parent)) = pipe();

  switch (fork()) {
  | `In_child =>
    Unix.close(write_parent);
    Unix.close(read_child);
    `In_child((`Write(write_child), `Read(read_parent)));
  | `In_parent(pid) =>
    Unix.close(read_parent);
    Unix.close(write_child);
    `In_parent((`Child_pid(pid), `Write(write_parent), `Read(read_child)));
  };
};

let select = (~read, ~write, ~except, ~timeout) => {
  switch (Unix.select(~read, ~write, ~except, ~timeout)) {
  | (read, write, except) => (`Read(read), `Write(write), `Except(except))
  };
};
