/**
  The type of a Policy.
  */
type t;

/**
  The default policy used with the scheduler.
  */
let default: unit => t;

/**
  The amount of workers the policy expects to run.
  */
let worker_count: t => int;
