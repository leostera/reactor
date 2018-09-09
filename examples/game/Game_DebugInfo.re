open Option;
open ReActor;
open Game_FFI;

let debugInfo = "debug_info";

type Message.t +=
  | Status(Event.data);

type state = {
  shouldUpdate: bool,
  lastKeyName: option(string),
  lastKeyCode: option(int),
  lastMouseClick: option((int, int)),
  lastMouseMove: option((int, int)),
  lastScreenSize: option((int, int)),
};

let handleMessage = state =>
  fun
  | Status(data) =>
    switch (data) {
    | Event.KeyDownData(keyName, keyCode) => {
        ...state,
        shouldUpdate: true,
        lastKeyName: Some(keyName),
        lastKeyCode: Some(keyCode),
      }
    | Event.MouseMoveData(x, y) => {
        ...state,
        shouldUpdate: true,
        lastMouseMove: Some((x, y)),
      }
    | Event.ClickData(x, y) => {
        ...state,
        shouldUpdate: true,
        lastMouseClick: Some((x, y)),
      }
    | Event.ResizeData(w, h) => {
        ...state,
        shouldUpdate: true,
        lastScreenSize: Some((w, h)),
      }
    | Event.NoData => state
    }
  | _ => state;

let render = state => {
  let {
    lastKeyName,
    lastKeyCode,
    lastMouseClick,
    lastMouseMove,
    lastScreenSize,
  } = state;
  let color = Canvas.RGBA(255, 255, 255, 0.8);
  let font = "20px Helvetica";
  let ops =
    Game_Renderer.[
      DrawText({j|Key Name: $lastKeyName|j}, font, Point2D(10, 20), color),
      DrawText({j|Key Code: $lastKeyCode|j}, font, Point2D(10, 40), color),
      DrawText({j|Click: $lastMouseClick|j}, font, Point2D(10, 60), color),
      DrawText({j|Move: $lastMouseMove|j}, font, Point2D(10, 80), color),
      DrawText({j|Screen: $lastScreenSize|j}, font, Point2D(10, 100), color),
    ];
  whereIs(Game_Renderer.name)
  >>| (pid => send(pid, Game_Renderer.Pipeline(ops)))
  |> ignore;
};

let loop: Process.f(state) =
  (env, state) => {
    let state' = env.recv() >>| handleMessage(state) <|> state;
    state'.shouldUpdate ? render(state') : ();
    Become({...state', shouldUpdate: false});
  };

let report = e =>
  whereIs(debugInfo) >>| (pid => send(pid, Status(e))) |> ignore;

let start = () =>
  spawn(
    loop,
    {
      shouldUpdate: true,
      lastKeyName: None,
      lastKeyCode: None,
      lastMouseClick: None,
      lastMouseMove: None,
      lastScreenSize: None,
    },
  )
  |> register(debugInfo);
