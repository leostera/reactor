# 🚀 reactor
> Native Actors for Reason and OCaml

**Status**: ALPHA. Ready for the adventurous 🤠

## Getting Started

If you're just starting out, have a look at the [Examples](/examples) section.
It includes examples in the form of _labs_ that you can play around with and
modify to get a better grasp of the core features of the library.

## Contributing

This project is setup and run with `esy`, so make sure you have that installed.

After that it's an `esy install` and `esy build` away.

Tests can be run with `esy dune runtest`.

Benchmarks can be run with `esy dune exec ./bench/{bench_name}.exe`

To run the examples you can use:

```
λ esy @examples/package.json dune exec --watch ./examples/sdl/gui_test.exe
```
