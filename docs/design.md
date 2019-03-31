# Reactor's Design

## Startup

When the application is set up for the first time, the current OS process
will become a __Node__. Its job will be to bootstrap, keep alive, and
coordinate messages across __Schedulers__. These Schedulers are separate OS
processes and have their own lifecycles.

```
+----------------+       +------------------+     +--------------+
|  Main Process  |   +==>+ Child Process #0 +-----+ Scheduler #0 |
+-----+----------+   |   +------------------+     +--------------+
      |              |
      |   +------+   |   +------------------+     +--------------+
      +---+ Node +<==+==>+ Child Process #1 +-----+ Scheduler #1 |
          +------+   |   +------------------+     +--------------+
                     |
                     |   +------------------+     +--------------+
                     +==>+ Child Process #2 +-----+ Scheduler #2 |
                     |   +------------------+     +--------------+
                     |
                     |   +------------------+     +--------------+
                     +==>+ Child Process #3 +-----+ Scheduler #3 |
                         +------------------+     +--------------+

```

All communication between processes is happening via Unix pipes or the
equivalent mechanism in other OS.

Because of the nature of Unix process forking, the Main process _becomes_ one
of it's children by copying the current memory. This means that we want to set
up the Node as early on as possible to avoid duplicating data we don't really
need in the Schedulers.

When a node is set up, it will create a _process pool_. Each child process will
be set up as a Scheduler, and, for each Scheduler set up, the Node will create
a _view_ of that Scheduler to keep track of them.

## Scheduling Loop

As soon as the Scheduler processes start up, they constantly loop through the
following list:

1. Check if the scheduler should halt, and if it shouldn't then:
   1. Check if it can read from the Node
   2. If there are tasks to read, read one and queue it
   3. If there are tasks in the queue, pick the next one and handle it
   4. Iterate on any open Lwt promises
   5. Loop back to 1.
2. If it should halt, it'll break the loop and continue the execution of the
   process to it's end. 

Step `1.3` can be broken down into:

* Handling tasks from the coordinator
* Handling tasks from itself — as a locality optimization, certain tasks that
  will be eventually handled within the same scheduler are simply not sent to
  the coordinator.
* Computing process reductions — this is what actually executes actors in the
  system.

The Node, acting as a coordinator of schedulers, goes through the following
list in a loop:

1. Should the node shut down? If it should, quit the loop, else continue.
2. If there's any commands to be read, queue them up
3. If there's any schedulers ready to receive commands and there are commands
   to send, send them to the schedulers.
