open Option;

open ReActor;
open Game_FFI;

module Renderer = {
  let name = "renderer";

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
    spawn(loop, initialState) |> register(name);
  };
};

module Pointer = {};

module Scene = {
  type state = {
    started: bool,
    color: Canvas.color,
    surface: Canvas.shape,
  };

  let repaint = (surface, color) =>
    where_is(Renderer.name)
    >>| (pid => send(pid, Renderer.Paint(surface, color)))
    |> ignore;

  let events = Event.[Click, MouseMove, KeyDown, Resize];

  let registerEvents = self =>
    where_is(EventHandler.name)
    >>| (
      pid =>
        events
        |> List.map(e => EventHandler.Subscribe(e, self))
        |> List.iter(send(pid))
    )
    |> ignore;

  let setup: Process.f(state) =
    (env, state) => {
      repaint(state.surface, state.color);
      registerEvents(env.self());
      Process.Become({...state, started: true});
    };

  let handleEvent = state =>
    fun
    | Event.KeyDownData(keyName, keyCode) => {
        Js.log3("key down", keyName, keyCode);
        Process.Become(state);
      }
    | Event.MouseMoveData(x, y) => {
        Js.log3("mouse move at", x, y);
        Process.Become(state);
      }
    | Event.ClickData(x, y) => {
        Js.log3("click at", x, y);
        Process.Become(state);
      }
    | Event.ResizeData(w, h) => {
        let state' = {...state, surface: Canvas.Rect(0, 0, w, h)};
        repaint(state'.surface, state'.color);
        Process.Become(state');
      }
    | Event.NoData => Process.Become(state);

  let handleMessage = state =>
    fun
    | EventHandler.Event(e) => handleEvent(state, e)
    | _ => Process.Become(state);

  let loop: Process.f(state) =
    (env, state) =>
      if (!state.started) {
        setup(env, state);
      } else {
        env.recv() >>| handleMessage(state) <|> Become(state);
      };

  let start = () =>
    spawn(
      loop,
      {
        started: false,
        color: Canvas.Hex(0x36454f),
        surface: Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
      },
    );
};

module Game = {
  let start = () => {
    let _input = EventHandler.start();
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
