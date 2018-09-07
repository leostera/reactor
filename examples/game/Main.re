open Option;

open ReActor;
open Game_FFI;

module Renderer = {
  type Message.t +=
    | Paint(Canvas.shape, Canvas.color);

  type t = {
    fpsCap: int,
    canvas: Canvas.context,
  };

  let handleMessage = (state, message) =>
    switch (message) {
    | Paint(shape, color) =>
      let _ = Canvas.fillRect(state.canvas, shape, color);
      Process.Become(state);
    | _ => Process.Become(state)
    };

  let loop: Process.f(t) =
    (env, state) => env.recv() >>| handleMessage(state) <|> Become(state);

  let start = () => {
    let initialState = {
      fpsCap: 30,
      canvas: DOM.elementById("game") |> Canvas.get2dContext,
    };
    spawn(loop, initialState) |> register("renderer");
  };
};

module Scene = {
  type state = {
    started: bool,
    color: Canvas.color,
    surface: Canvas.shape,
  };

  let setup: Process.f(state) =
    (_env, state) =>
      state.started ?
        Process.Become(state) :
        (
          switch (where_is("renderer")) {
          | None => Process.Terminate
          | Some(renderer) =>
            send(renderer, Renderer.Paint(state.surface, state.color));
            Process.Become({...state, started: true});
          }
        );

  let start = () =>
    spawn(
      setup,
      {
        started: false,
        color: Canvas.Hex(0x36454f),
        surface: Canvas.Rect(0, 0, 500, 500),
      },
    );
};

module Game = {
  let start = () => {
    let _input = Game_InputHandler.start("screen");
    let _renderer = Renderer.start();
    let _scene = Scene.start();
    ();
  };
};

trace({
  matcher: (_pid, _message) => true,
  timeout: 5_000_000_000,
  handler: Js.log,
});

Game.start();
