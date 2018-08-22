type t; /* abstract worker type */

[@bs.deriving abstract]
type message('a) = {data: 'a};

type worker_type =
  | ModuleWorker;

let worker_type_to_string =
  fun
  | ModuleWorker => "module";

[@bs.val] external self: t = "self";

module FFI = {
  [@bs.deriving abstract]
  type opts = {
    [@bs.as "type"]
    worker_type: string,
  };

  [@bs.new] external __unsafe_new_Worker: (string, opts) => t = "Worker";

  [@bs.set]
  external __unsafe_onMessage: ('a, 'm => unit) => unit = "onmessage";

  [@bs.send] external __unsafe_postMessage: (t, 'm) => unit = "postMessage";
};

let make: (string, worker_type) => t =
  (src, worker_type) =>
    FFI.opts(~worker_type=worker_type_to_string(worker_type))
    |> FFI.__unsafe_new_Worker(src);

let onMessage: (message('m) => unit, t) => t =
  (handler, worker) => {
    FFI.__unsafe_onMessage(worker, handler);
    worker;
  };

let postMessage: ('m, t) => unit =
  (message, worker) => FFI.__unsafe_postMessage(worker, message);
