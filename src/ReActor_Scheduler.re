open ReActor_Core;

let defaultWorker = "/lib/es6_global/src/ReActor_Worker.bs.js";

type worker = {
  id: int,
  worker: Worker.t(instruction, reply(ReActor_Worker.t)),
};

type t = {
  tasks: list(unit => unit),
  maxConcurrency: int,
  workers: list(worker),
  checkupFreq: int,
};

module Handlers = {
  let handleWorkerMessage: Worker.message(reply(ReActor_Worker.t)) => unit =
    message =>
      switch (Worker.dataGet(message)) {
      | WorkerStatus({id, proc_count}) =>
        Js.log({j|Worker($id) [ {proc_count,$proc_count} ]|j})
      };
};

module Workers = {
  let ping = workers =>
    workers
    |> List.map(({worker}) => worker)
    |> List.iter(Worker.postMessage(Info));

  let make = (count, workerSrc) => {
    let rec b = (n, acc) =>
      switch (n, acc) {
      | (0, acc) => acc
      | (n, acc) =>
        let worker =
          Worker.(
            make(workerSrc, ModuleWorker)
            |> onMessage(Handlers.handleWorkerMessage)
          );

        b(n - 1, [{id: n, worker}, ...acc]);
      };
    b(count, []);
  };
};

let rec loop = scheduler => {
  let _ =
    Runtime.defer(
      () => {
        Workers.ping(scheduler.workers);
        loop(scheduler);
      },
      scheduler.checkupFreq,
    );
  ();
};

let make = (~checkupFreq) => {
  let workerCount = Window.hardwareConcurrency - 1;
  {
    tasks: [],
    maxConcurrency: workerCount,
    workers: Workers.make(workerCount, defaultWorker),
    checkupFreq,
  };
};
