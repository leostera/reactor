/**
  # Basic Message Passing

  In this lab we will learn how to send messages to a process, and how that
  process can consume those messages.

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
  Great! We are ready to start now.

  Messages in Reactor are, deep inside, just strings. This seems like an
  overlook, but ultimately its a decision taken to allow for maximum
  flexibility while maintining the implementation of the library relatively
  simple.

  This means that it is completely up to you to use vanilla strings such as
  "hello" as messages, or if you want to pass around JSON, MsgPack, Protobuf, or
  OCaml Marshal bytes.

  For now we will assume that our messages are plain old strings, but we will
  look into custom encoders later on.

  Let's begin with a familiar process, but this time we will save it's _pid_.
  */
let pid = spawn((_ctx, state) => `Become(state), 0);

/**
  Cool. This pid is needed to send messages to this process, and to send
  messages we can use the `send` function, or the `<-` operator.

  Let's send two messages, "hello", and "world":
 */
send(pid, "hello");
pid <- "world";

/**
  That was easy. Unfortunately not much has happened, because our process isn't
  really checking for messages.

  Every process, as you may have seen, takes a `ctx` variable. We have ignored
  it up until now, but it's time to access one of the functions it exposes:
  `ctx.recv()`.

  The `recv` function will return an optional message. This value will be
  `None` if there are no new messages available for our process to consume, or
  `Some(msg)`, where `msg` will be the next message in the queue.

  It is worth noting that calling `recv` will effectively _consume_ a message
  out of the mailbox.

  Lets create a new process that will log every message it receives, and send it
  the same 2 messages:
 */
let pid' =
  spawn(
    (ctx, state) => {
      switch (ctx.recv()) {
      | Some(msg) => Logs.app(m => m("Received: %s", msg))
      | None => ()
      };
      `Become(state);
    },
    0,
  );
pid' <- "hello";
pid' <- "world";

/**
  Magnificent. This new process actually checks for messages and prints them
  out.

  Thanks to the ability to pattern match, we can handle messages quite
  declaratively. For example, this next process will react only to the
  `"terminate"` message and terminate the system:
 */
let pid'' =
  spawn(
    (ctx, state) =>
      switch (ctx.recv()) {
      | Some("pls exit") =>
        /** replace this with a call to `exit()` */ `Terminate
      | _ => `Become(state)
      },
    0,
  );
pid'' <- "what";
pid'' <- "pls exit";

/**
  Lastly, after we have set up our application, we need to tell Reactor that it
  should start executing our processes.

  We do this with the call below, which will block until some process calls
  `exit`.
 */
Reactor.Scheduler.run();

/**
  And that's it for today! You're now ready to create thousands of processes
  that will send messages to each other.
  */;
