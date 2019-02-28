module type Base = {type t;};

/** Common message encoding/decoding */
module Make = (M: Base) => {
  let encode: M.t => string = x => Marshal.to_string(x, [Marshal.Closures]);
  let decode: string => M.t = x => Marshal.from_string(x, 0);
};
