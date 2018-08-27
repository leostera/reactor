module DOM = {
  type node;

  [@bs.val] [@bs.scope "document"]
  external __unsafe_get_by_id: string => node = "getElementById";

  [@bs.set]
  external __unsafe_set_inner_text: (node, string) => unit = "innerText";

  let getElementById = __unsafe_get_by_id;

  let withInnerText = __unsafe_set_inner_text;
};

module Date = {
  [@bs.val] [@bs.scope "Date"] external __unsafe_now: unit => int = "now";

  let now = __unsafe_now;
};

module Performance = {
  [@bs.val] [@bs.scope "performance"]
  external __unsafe_now: unit => int = "now";

  let now = __unsafe_now;
};

module Monotonic = {
  let __monotonic_counter = ref(0);
  let next = () => {
    let last = __monotonic_counter^;
    __monotonic_counter := __monotonic_counter^ + 1;
    last;
  };
};

module UInt16Array = {
  type t;

  [@bs.new] external __unsafe_make: int => t = "Uint16Array";

  [@bs.val] [@bs.scope "Array"]
  external __unsafe_to_array: t => array(int) = "from";

  let make = __unsafe_make;

  let to_array = __unsafe_to_array;
};

module Number = {
  [@bs.send] external __unsafe_to_string: (int, int) => string = "toString";

  let to_string = (radix, value) => __unsafe_to_string(value, radix);
};

module Random = {
  [@bs.val] [@bs.scope "self.crypto"]
  external __unsafe_fill_buffer: UInt16Array.t => unit = "getRandomValues";

  [@bs.val] [@bs.scope "Math"]
  external __unsafe_random: unit => float = "random";

  let inRange = (min, max) =>
    min + int_of_float(__unsafe_random() *. float_of_int(max));

  let random = n => {
    let buffer = UInt16Array.make(n);
    let () = __unsafe_fill_buffer(buffer);
    buffer;
  };

  let asHex = (buff, ~length) =>
    buff
    |> UInt16Array.to_array
    |> Array.map(Number.to_string(16))
    |> Array.fold_left((a, b) => {j|$a$b|j}, "")
    |> (x => String.sub(x, 0, length));

  let shortId = () => random(7)->asHex(~length=7);
};

let make_list = (n, f) => Array.make(n, 0) |> Array.to_list |> List.map(f);
