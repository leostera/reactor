open Option;

open ReActor;
open ReActor_Utils;
open Game_FFI;

module Cursor = {
  type status =
    | Click
    | Normal;

  type state = {
    x: int,
    y: int,
    status,
  };

  type Message.t +=
    | Move(int, int)
    | Click(int, int);

  let handleMessage = (state, message) =>
    switch (message) {
    | Move(x, y) => {x, y, status: Normal}
    | Click(x, y) => {x, y, status: Click}
    | _ => state
    };

  let render = ({x, y}) =>
    whereIs(Game_Renderer.name)
    >>| (
      pid =>
        send(
          pid,
          Game_Renderer.(
            Pipeline([
              DrawCircle(
                Point2D(x, y),
                10,
                Canvas.RGBA(255, 255, 255, 0.7),
              ),
            ])
          ),
        )
    )
    |> ignore;

  let loop: Process.f(state) =
    (env, state) => {
      let state' = env.recv() >>| handleMessage(state) <|> state;

      switch (state'.status) {
      | Click => render(state')
      | _ => ()
      };

      Become(state');
    };

  let start = () => spawn(loop, {x: 0, y: 0, status: Normal});
};

module Scene = {
  type status =
    | ShouldSetup
    | Started;

  type state = {
    status,
    cursor: Pid.t,
    color: Canvas.color,
    surface: Canvas.shape,
  };

  let repaint = (surface, color) =>
    whereIs(Game_Renderer.name)
    >>| (
      pid => send(pid, Game_Renderer.(Pipeline([DrawRect(surface, color)])))
    )
    |> ignore;

  let events = Event.[Click, MouseMove, KeyDown, Resize];

  let registerEvents = self =>
    whereIs(EventHandler.name)
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
      Process.Become({...state, status: Started});
    };

  let handleEvent = (state, event) => {
    Game_DebugInfo.report(event);
    switch (event) {
    | Event.ResizeData(w, h) =>
      let state' = {...state, surface: Canvas.Rect(0, 0, w, h)};
      repaint(state'.surface, state'.color);
      Process.Become(state');
    | Event.MouseMoveData(x, y) =>
      send(state.cursor, Cursor.Move(x, y));
      Process.Become(state);
    | Event.ClickData(x, y) =>
      send(state.cursor, Cursor.Click(x, y));
      Process.Become(state);
    | _ => Process.Become(state)
    };
  };

  let handleMessage = state =>
    fun
    | EventHandler.Event(e) => handleEvent(state, e)
    | _ => Process.Become(state);

  let loop: Process.f(state) =
    (env, state) =>
      switch (state.status) {
      | ShouldSetup => setup(env, state)
      | _ => env.recv() >>| handleMessage(state) <|> Become(state)
      };

  let start = () =>
    spawn(
      loop,
      {
        status: ShouldSetup,
        cursor: Cursor.start(),
        color: Canvas.Hex(0x36454f),
        surface: Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
      },
    );
};

module Game = {
  let start = () => {
    let _debugInfo = Game_DebugInfo.start();
    let _input = EventHandler.start();
    let _renderer = Game_Renderer.start();
    let _scene = Scene.start();
    ();
  };
};

trace({
  matcher: (_pid, _message) => true,
  timeout: 5_000_000_000,
  handler: e => Js.log2(Date.now(), e),
});

Game.start();
