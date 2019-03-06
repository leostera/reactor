/**
  Hello!

  In this first lab we will learn how to spawn processes and what they can do.
  It's a pretty straightforward lab, as we will introduce only spawning,
  becoming, and exiting the Reactor system.

  Before, I'll do some hand-wavy log setup ✨
  */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.App));
Logs.set_reporter(Logs_fmt.reporter());

/**
  Ta-da! Okay, last thing to do is to actually set up the Reactor system. For
  now we just need to know that this setup needs to be in place _before_ our
  application starts.

  As a bonus we will open the [!Reactor.System] module to get access to a few
  handy functions.
*/
Reactor.Scheduler.(Policy.default() |> setup);

open Reactor.System;

/**
  Great! We are ready to start now, so lets do that by defining what is
  spawning and becoming.

  *Spawning* is the act of creating a new Process. To spawn a process, we need
  a function that will define the behavior of the process, and an initial
  state.

  *Becoming* is what Processes do to change their state, and it behaves like a
  tail-recursive call. This means that state is not mutable, but rather that a
  process must compute it's next state, and it will essentially call itself
  with it.

  The example below will spawn a process that simply keeps state and never
  terminates.
  */
let _ = spawn((_ctx, state) => `Become(state), 0);

/**
  Yes, it's a pretty useless process, but it shows how easy it is to spawn them
  just by using a lambda :) The type of the state in this case is inferred from
  the initial state (`0`). The return value is being ignored, and we will see
  how to use it later on.

  The way this works is that whatever a process decides to __become__, will be
  the next iterations state.

  Here's another example that changes its state on each iteration. If the state
  is `true`, it becomes `false`, and viceversa.
 */
let _ =
  spawn(
    (_ctx, state) =>
      switch (state) {
      | true => `Become(false)
      | _ => `Become(true)
      },
    true,
  );

/**
  Because the body of a process is just a function, you can do many things in
  it too, including side-effects!
 */
let _ =
  spawn(
    (_ctx, state) =>
      /** uncomment this to see the logs! */
      /** Logs.app(m => m("Current state: %s", state)); */
      `Become(state),
    "hello",
  );

/**
   Now that we've seen a little about how to spawn processes and evolve them,
   lets try changing the process below to call `exit()` when the counter
   reaches zero.

   `exit` is a function that normally is available in Reason/OCaml native to
   exit the current program. In this case, its been shadowed so that it
   performs a reasonable cleanup before continuing.
 */
let _ = spawn((_ctx, counter) => `Become(counter - 1), 100);

/**
  Lastly, after we have set up our application, we need to tell Reactor that it
  should start executing our processes.

  We do this with the call below, which will block until some process calls
  `exit`.
 */
Reactor.Scheduler.run();

/**
  And that's it for today! You're now ready to create thousands of processes
  that will evolve as they carry out tasks and eventually finish.
  */;
