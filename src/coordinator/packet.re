let header_size = 32;

let encode: Bytecode.t => bytes =
  data => {
    let raw_data = Marshal.to_bytes(data, [Marshal.Closures]);
    let data_size = raw_data |> Bytes.length;
    let raw_size = Marshal.to_bytes(data_size, []);
    let buf = Buffer.create(data_size + header_size);
    switch (header_size - (raw_size |> Bytes.length)) {
    | 0 => ()
    | n => Buffer.add_bytes(buf, Bytes.create(n) |> Bytes.map(_ => ' '))
    };
    Buffer.add_bytes(buf, raw_size);
    Buffer.add_bytes(buf, raw_data);
    buf |> Buffer.to_bytes;
  };

let read_from_pipe = (`Read(pipe)) => {
  let raw_size = Platform.Process.read(pipe, ~len=header_size);
  let data_size = raw_size |> Bytes.trim |> (x => Marshal.from_bytes(x, 0));
  let raw_data = Platform.Process.read(pipe, ~len=data_size);
  Marshal.from_bytes(raw_data, 0);
};
