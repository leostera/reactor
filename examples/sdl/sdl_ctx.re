/** SDL Context Process */
open Reactor.System;
open Tsdl;

type state = {
  window: option(Sdl.window),
  renderer: option(Sdl.renderer),
};

module M = {
  type t =
    | Run_pipeline((Sdl.window, Sdl.renderer) => unit)
    | Init;

  include Message_codec.Make({
    type nonrec t = t;
  });

  let init: Reactor.Pid.t => unit = pid => pid <- (Init |> encode);

  let run_pipeline: (Reactor.Pid.t, (Sdl.window, Sdl.renderer) => unit) => unit =
    (pid, pipeline) => pid <- (Run_pipeline(pipeline) |> encode);
};

let handle_message = (self, msg, state) => {
  switch (msg, state.window, state.renderer) {
  | (M.Init, None, _) =>
    Logs.app(m =>
      m("%s: Initializing SDL systems...", self |> Reactor.Pid.to_string)
    );
    switch (Sdl.init(Sdl.Init.video)) {
    | Error(`Msg(e)) =>
      Logs.err(m => m("[Sdl_ctx.handle_message] Init error: %s", e));
      `Terminate;
    | Ok () =>
      let window =
        Sdl.create_window(~w=640, ~h=480, "SDL OpenGL", Sdl.Window.opengl);
      switch (window) {
      | Error(`Msg(e)) =>
        Logs.err(m =>
          m("[Sdl_ctx.handle_message] Create window error: %s", e)
        );
        `Terminate;
      | Ok(window) =>
        switch (Sdl.create_renderer(window)) {
        | Error(`Msg(e)) =>
          Logs.err(m =>
            m("[Sdl_ctx.handle_message] Create window error: %s", e)
          );
          `Terminate;
        | Ok(renderer) =>
          Logs.app(m =>
            m("%s: Initialization Complete!", self |> Reactor.Pid.to_string)
          );

          `Become({window: Some(window), renderer: Some(renderer)});
        }
      };
    };
  | (M.Run_pipeline(pipeline), Some(window), Some(renderer)) =>
    pipeline(window, renderer);
    `Become(state);
  | _ => `Become(state)
  };
};

let handle_tick = state => {
  switch (state.window) {
  | Some(w) =>
    let e = Sdl.Event.create();
    let _ = Sdl.push_event(e);
    switch (Sdl.wait_event(Some(e))) {
    | Error(`Msg(err)) =>
      Logs.err(m => m("[Sdl_ctx.handle_tick] Could not wait event: %s", err));
      `Terminate;
    | Ok () =>
      switch (Sdl.Event.(enum(get(e, typ)))) {
      | `Key_down
      | `Quit =>
        Sdl.destroy_window(w);
        Sdl.quit();
        `Terminate;
      | _ => `Become(state)
      }
    };
  | _ => `Become(state)
  };
};

let loop: Reactor.Process.task(state) =
  (ctx, state) =>
    switch (ctx.recv()) {
    | None => handle_tick(state)
    | Some(msg) => handle_message(ctx.self(), msg |> M.decode, state)
    };

let start = () => spawn(loop, {window: None, renderer: None});
