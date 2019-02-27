/**
  The type of a Policy.
  */
type t;

/**
  The default policy used with the scheduler. Will use as many workers as CPU
  cores available.
  */
let default: unit => t;

/**
  Custom policy where all options are configurable.
  */
let custom: (~worker_count: int) => t;

/**
  The amount of workers the policy expects to run.
  */
let worker_count: t => int;
