type t = {
  /** The amount of schedulers to use */
  schedulers: int,
};

let default = () => {schedulers: Platform.cpu_count()};

let custom = (~scheduler_count) => {schedulers: scheduler_count};

let scheduler_count = t => t.schedulers;
