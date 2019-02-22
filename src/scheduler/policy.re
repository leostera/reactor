type t = {
  /** The amount of workers to use */
  workers: int,
};

let default = () => {workers: Platform.Cpu.count()};

let worker_count = t => t.workers;
