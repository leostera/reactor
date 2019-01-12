open Lwt.Infix;

let work_queue = Queue.create();

module Scheduler =
  Reactor.Scheduler.Make({
    let defer = t => {
      Queue.add(t, work_queue);
    };

    let run = _scheduler => {
      let rec consume = () => {
        switch (Queue.pop(work_queue)) {
        | exception Queue.Empty => Lwt.return_unit
        | work => work() |> Lwt.return >>= consume
        };
      };
      consume() |> Lwt_main.run;
    };
  });
