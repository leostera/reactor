module DOM = {
  type node;

  [@bsval] [@bsscope "document"]
  external __unsafe_get_by_id: string => node = "getElementById";

  [@bsset]
  external __unsafe_set_inner_text: (node, string) => unit = "innerText";

  let getElementById = __unsafe_get_by_id;

  let withInnerText = __unsafe_set_inner_text;
};

module Date = {
  [@bsval] [@bsscope "Date"] external __unsafe_now: unit => int = "now";

  let now = __unsafe_now;
};

module Performance = {
  [@bsval] [@bsscope "performance"]
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

  [@bsnew] external __unsafe_make: int => t = "Uint16Array";

  [@bsval] [@bsscope "Array"]
  external __unsafe_to_array: t => array(int) = "from";

  let make = __unsafe_make;

  let to_array = __unsafe_to_array;
};

module Number = {
  [@bssend] external __unsafe_to_string: (int, int) => string = "to_string";

  let to_string = (radix, value) => __unsafe_to_string(value, radix);
};

module Random = {
  [@bsval] [@bsscope "selfcrypto"]
  external __unsafe_fill_buffer: UInt16Arrayt => unit = "getRandomValues";

  [@bsval] [@bsscope "Math"]
  external __unsafe_random: unit => float = "random";

  let inRange = (min, max) =>
    min + int_of_float(__unsafe_random() * float_of_int(max));

  let random = n => {
    let buffer = UInt16Arraymake(n);
    let () = __unsafe_fill_buffer(buffer);
    buffer;
  };

  let asHex = (buff, ~length) =>
    buff
    |> UInt16Arrayto_array
    |> Arraymap(Numberto_string(16))
    |> Arrayfold_left((a, b) => {j|$a$b|j}, "")
    |> (x => Stringsub(x, 0, length));

  let shortId = () => random(7)->asHex(~length=7);
};

let make_list = (n, f) => Arraymake(n, 0) |> Arrayto_list |> Listmap(f);
