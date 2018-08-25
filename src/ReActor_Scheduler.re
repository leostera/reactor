open ReActor_Process;
open ReActor_Utils;

type instruction =
  | Info
  | Spawn;

type reply('a) =
  | SchedulerInfo('a);

type kind =
  | Main;

type t = {
  id: string,
  proc_count: int,
};

let leastBusy: list(ref(t)) => ref(t) = workers => List.nth(workers, 0);

let make_workerThread = () =>
  FFI_WebWorker.make(
    ~src="/lib/es6_global/src/ReActor_WebWorker.bs.js",
    ~worker_type=FFI_WebWorker.ModuleWorker,
  );

let make = () => {id: Random.(random(7)->asHex(~length=7)), proc_count: 0};

let send: (instruction, ref(t)) => unit =
  (msg, worker) =>
    switch (msg) {
    | Info => Js.log(worker^)
    | Spawn =>
      let pid = pid(0, worker^.proc_count, 0);
      Js.log({j| Created Process($pid) |j});
    };
