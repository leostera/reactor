type t = {
  /** The amount of workers to use */
  workers: int,
};

let default = () => {workers: Platform.Cpu.count()};

let custom = (~worker_count) => {workers: worker_count};

let worker_count = t => t.workers;
