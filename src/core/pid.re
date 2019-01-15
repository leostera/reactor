type t = int;

let make = n => n;

let to_string = t => string_of_int(t);

let equal = (a, b) => String.equal(string_of_int(a), string_of_int(b));

let next = t => t + 1;
