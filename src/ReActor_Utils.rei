module DOM: {
  type node;
  let getElementById: string => node;
  let withInnerText: (node, string) => unit;
};

module Date: {let now: unit => int;};

module Performance: {let now: unit => int;};

module Monotonic: {
  let __monotonic_counter: ref(int);
  let next: unit => int;
};

module UInt16Array: {
  type t;
  let make: int => t;
  let to_array: t => array(int);
};

module Number: {let to_string: (int, int) => string;};

module Random: {
  let inRange: (int, int) => int;
  let random: int => UInt16Array.t;
  let asHex: (UInt16Array.t, ~length: int) => string;
  let shortId: unit => string;
};

let make_list: (int, int => 'a) => list('a);
