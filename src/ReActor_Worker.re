open ReActor_Core;
open ReActor_Utils;

type t = {
  id: string,
  proc_count: int,
  processes: list(process),
};

let id = Random.(random(7)->asHex(~length=7));
let this = ref({id, processes: [], proc_count: 0});

module Handlers = {
  let handle_info = () =>
    Worker.self |> Worker.postMessage(WorkerStatus(this^));

  let handle_spawn = () => Js.log({j|Spawning|j});

  let onMessage = message =>
    switch (Worker.dataGet(message)) {
    | Info => handle_info()
    | Spawn => handle_spawn()
    };
};

let rec loop = worker =>
  Runtime.defer(
    () => {
      worker.processes |> List.iter(p => p.f());
      loop(worker);
    },
    4,
  );

loop(this^);

Worker.self |> Worker.onMessage(Handlers.onMessage);
