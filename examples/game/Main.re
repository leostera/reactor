open Option;

open ReActor;
open Game_FFI;

module Scene = {
  type state = {
    started: bool,
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
      Process.Become({...state, started: true});
    };

  let handleEvent = state =>
    fun
    | Event.KeyDownData(keyName, keyCode) as e => {
        Game_DebugInfo.report(e);
        Js.log3("key down", keyName, keyCode);
        Process.Become(state);
      }
    | Event.MouseMoveData(x, y) as e => {
        Game_DebugInfo.report(e);
        Js.log3("mouse move at", x, y);
        Process.Become(state);
      }
    | Event.ClickData(x, y) as e => {
        Game_DebugInfo.report(e);
        Js.log3("click at", x, y);
        Process.Become(state);
      }
    | Event.ResizeData(w, h) as e => {
        Game_DebugInfo.report(e);
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
  handler: Js.log,
});

Game.start();
