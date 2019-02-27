let count = () =>
  try (
    switch (Sys.os_type) {
    | "Win32" => Sys.getenv("NUMBER_OF_PROCESSORS") |> int_of_string
    | _ =>
      let ic = Unix.open_process_in("getconf _NPROCESSORS_ONLN");
      ic |> input_line |> int_of_string;
    }
  ) {
  | _ => 1
  };
