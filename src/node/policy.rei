/**
  The type of a Policy.
  */
type t;

/**
  The default policy used with the scheduler. Will use as many schedulers as CPU
  cores available.
  */
let default: unit => t;

/**
  Custom policy where all options are configurable.
  */
let custom: (~scheduler_count: int) => t;

/**
  The amount of schedulers the policy expects to run.
  */
let scheduler_count: t => int;
