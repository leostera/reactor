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

