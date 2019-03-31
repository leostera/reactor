module Matcher = {
  let pid = line => {
    let parts = String.split_on_char(' ', line) |> Array.of_list;
    parts[2];
  };
};

module IO = {
  let read_lines = () => {
    let rec read = acc => {
      switch (input_line(stdin)) {
      | exception _ => acc |> List.rev
      | line => read([line, ...acc])
      };
    };
    read([]);
  };
};

module Bucket = {
  module Table =
    Hashtbl.Make({
      type t = string;
      let equal = String.equal;
      let hash = Hashtbl.hash;
    });

  type t = Table.t(list(string));

  let create: int => t = Table.create;

  let add = (t, line) => {
    let pid = Matcher.pid(line);
    let lines_for_pid =
      switch (Table.find_opt(t, pid)) {
      | Some(v) => v
      | None => []
      };
    Table.replace(t, pid, [line, ...lines_for_pid]);
  };

  let dump = t => {
    Table.to_seq(t)
    |> List.of_seq
    |> List.sort(((p1, _), (p2, _)) => String.compare(p1, p2))
    |> List.iter(((pid, lines)) => {
         Printf.printf("Process %s: \n", pid);
         lines |> List.rev |> List.iter(line => Printf.printf("%s\n%!", line));
         Printf.printf("\n");
       });
  };
};

let () = {
  let lines = IO.read_lines();
  let bucket = Bucket.create(1024);
  lines |> List.iter(Bucket.add(bucket));
  Bucket.dump(bucket);
};
