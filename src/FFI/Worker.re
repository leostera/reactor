type t('a, 'b); /* abstract worker type */

[@bs.deriving abstract]
type message('a) = {data: 'a};

type worker_type =
  | ModuleWorker;

let worker_type_to_string =
  fun
  | ModuleWorker => "module";

[@bs.val] external self: t('a, 'b) = "self";

module FFI = {
  [@bs.deriving abstract]
  type opts = {
    [@bs.as "type"]
    worker_type: string,
  };

  [@bs.new]
  external __unsafe_new_Worker: (string, opts) => t('a, 'b) = "Worker";

  [@bs.set]
  external __unsafe_onMessage: (t('a, 'b), message('m) => unit) => unit =
    "onmessage";

  [@bs.send]
  external __unsafe_postMessage: (t('a, 'b), 'm) => unit = "postMessage";
};

let make: (string, worker_type) => t('a, 'b) =
  (src, worker_type) =>
    FFI.opts(~worker_type=worker_type_to_string(worker_type))
    |> FFI.__unsafe_new_Worker(src);

let onMessage: (message('b) => unit, t('a, 'b)) => t('a, 'b) =
  (handler, worker) => {
    FFI.__unsafe_onMessage(worker, handler);
    worker;
  };

let postMessage: ('a, t('a, 'b)) => unit =
  (message, worker) => FFI.__unsafe_postMessage(worker, message);
