open Reactor.System;

/** Logging setup */
Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Error));
Logs.set_reporter(Logs_fmt.reporter());

/** System setup */
Reactor.Scheduler.(Policy.default() |> setup);

module Screen_cleaner = {
  open Tsdl;

  type color = (int, int, int);
  type state = {
    sdl_ctx: Reactor.Pid.t,
    color,
  };

  let do_render = (sdl_ctx, (r, g, b)) => {
    let clear_screen = (_window, renderer) => {
      Sdl.set_render_draw_color(renderer, r, g, b, 255) |> ignore;
      Sdl.render_clear(renderer) |> ignore;
      Sdl.render_present(renderer) |> ignore;
    };
    Sdl_ctx.M.run_pipeline(sdl_ctx, clear_screen);
  };

  let next_color = (r, g, b) => {
    switch (r, g, b) {
    | (255, 255, 255) => (0, 0, 0)
    | (255, 255, _) => (r, g, b + 1)
    | (255, _, _) => (r, g + 1, b)
    | (_, _, _) => (r + 1, g, b)
    };
  };

  let loop: Reactor.Process.task(state) =
    (_ctx, state) => {
      do_render(state.sdl_ctx, state.color);
      let (r, g, b) = state.color;
      `Become({sdl_ctx: state.sdl_ctx, color: next_color(r, g, b)});
    };

  let start = (sdl_ctx, (r, g, b)) => {
    spawn(loop, {sdl_ctx, color: (r, g, b)});
  };
};

let sdl_ctx = Sdl_ctx.start();
Sdl_ctx.M.init(sdl_ctx);
let _ = Screen_cleaner.start(sdl_ctx, (0, 0, 0));

Reactor.Scheduler.run();
