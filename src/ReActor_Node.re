open ReActor_Process;
open ReActor_Utils;

type clean;

type looping;

type t('status) = {
  tasks: list(unit => unit),
  maxConcurrency: int,
  workers: list(ref(ReActor_Scheduler.t)),
  checkupFreq: int,
};

module Schedulers = {
  let handleMessage: ReActor_Scheduler.reply(ReActor_Scheduler.t) => unit =
    message =>
      switch (message) {
      | SchedulerInfo({id, proc_count}) =>
        Js.log({j|Scheduler($id) [ {proc_count,$proc_count} ]|j})
      };

  let ping = workers => workers |> List.iter(ReActor_Scheduler.send(Info));

  let make = _count => [
    ref(ReActor_Scheduler.make()),
    /*
     ...make_list(count - 1, _ => ReActor_Scheduler.(make(Scheduler))),
     */
  ];
};

let rec run = s => {
  let _ =
    FFI_Runtime.defer(
      () => {
        Schedulers.ping(s.workers);
        run(s);
      },
      s.checkupFreq,
    );
  {
    tasks: s.tasks,
    maxConcurrency: s.maxConcurrency,
    workers: s.workers,
    checkupFreq: s.checkupFreq,
  };
};

let make = (~checkupFreq, ~maxConcurrency) => {
  tasks: [],
  maxConcurrency,
  workers: Schedulers.make(maxConcurrency),
  checkupFreq,
};

let spawn = ({workers}, ()) => {
  let w = workers |> ReActor_Scheduler.leastBusy;
  let w': ReActor_Scheduler.t = {id: w^.id, proc_count: w^.proc_count + 1};
  let pid = (0, w'.proc_count, 0);
  w := w';
  w |> ReActor_Scheduler.send(Spawn);
  pid;
};
