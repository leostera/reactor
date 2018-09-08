open Option;
open ReActor;
open Game_FFI;

let debugInfo = "debug_info";

type Message.t +=
  | Status(Event.data);

type state = {
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
        lastKeyName: Some(keyName),
        lastKeyCode: Some(keyCode),
      }
    | Event.MouseMoveData(x, y) => {...state, lastMouseMove: Some((x, y))}
    | Event.ClickData(x, y) => {...state, lastMouseClick: Some((x, y))}
    | Event.ResizeData(w, h) => {...state, lastScreenSize: Some((w, h))}
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
  let ops =
    Game_Renderer.[
      Fill(Canvas.Hex(0)),
      DrawText({j|Key Name: $lastKeyName|j}, Point2D(10, 10), color),
      DrawText({j|Key Code: $lastKeyCode|j}, Point2D(10, 20), color),
      DrawText({j|Click: $lastMouseClick|j}, Point2D(10, 30), color),
      DrawText({j|Move: $lastMouseMove|j}, Point2D(10, 40), color),
      DrawText({j|Screen: $lastScreenSize|j}, Point2D(10, 50), color),
    ];
  whereIs(Game_Renderer.name)
  >>| (pid => send(pid, Game_Renderer.Pipeline(ops)))
  |> ignore;
};

let loop: Process.f(state) =
  (env, state) => {
    let state' = env.recv() >>| handleMessage(state) <|> state;
    render(state');
    Suspend(16, state');
  };

let report = e =>
  whereIs(debugInfo) >>| (pid => send(pid, Status(e))) |> ignore;

let start = () =>
  spawn(
    loop,
    {
      lastKeyName: None,
      lastKeyCode: None,
      lastMouseClick: None,
      lastMouseMove: None,
      lastScreenSize: None,
    },
  )
  |> register(debugInfo);
