let header_size = 8;

let encode = data => {
  let raw_data = Marshal.to_bytes(data, [Marshal.Closures]);
  let data_size = raw_data |> Bytes.length;
  let raw_size = data_size |> Int32.of_int |> Int32.to_string;
  let buf = Buffer.create(data_size + header_size);
  switch (header_size - (raw_size |> String.length)) {
  | 0 => ()
  | n => Buffer.add_bytes(buf, Bytes.create(n) |> Bytes.map(_ => '0'))
  };
  Buffer.add_string(buf, raw_size);
  Buffer.add_bytes(buf, raw_data);
  buf |> Buffer.to_bytes |> Bytes.to_string;
};

let read_from_pipe = (`Read(pipe)) => {
  let raw_size = Platform.Process.read(pipe, ~len=header_size);
  let data_size =
    switch (Int32.of_string_opt(raw_size)) {
    | None =>
      Logs.info(m =>
        m("Something went wrong reading the size of the packet!")
      );
      0;
    | Some(x) => x |> Int32.to_int
    };
  let raw_data = Platform.Process.read(pipe, ~len=data_size);
  Marshal.from_string(raw_data, 0);
};
