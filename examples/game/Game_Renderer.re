open Option;
open ReActor;
open Game_FFI;

let name = "renderer";

type ops =
  | Fill(Canvas.color)
  | DrawCircle(Canvas.point, int, Canvas.color)
  | DrawText(string, string, Canvas.point, Canvas.color)
  | DrawRect(Canvas.shape, Canvas.color);

type Message.t +=
  | Pipeline(list(ops))
  | Flush;

type status =
  | Flushed
  | CanFlush;

type t = {
  status,
  buffer: Canvas.canvas,
  buffer_ctx: Canvas.context,
  screen: Canvas.context,
  ops: list(ops),
};

let paint = state =>
  fun
  | Fill(_) => {
      let _ =
        Canvas.fillRect(
          state.buffer_ctx,
          Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
          Canvas.Hex(0x36454f),
        );
      ();
    }
  | DrawCircle(origin, radius, color) => {
      let _ = Canvas.beginPath(state.buffer_ctx);
      let _ =
        Canvas.arc(
          state.buffer_ctx,
          origin,
          radius,
          0.0,
          6.283185307179586,
          Canvas.Clockwise,
        );
      let _ = Canvas.fillStyle(state.buffer_ctx, color);
      let _ = Canvas.fill(state.buffer_ctx);
      ();
    }
  | DrawText(text, font, origin, color) => {
      let _ = Canvas.fillText(state.buffer_ctx, text, font, origin, color);
      ();
    }
  | DrawRect(rect, color) => {
      let _ = Canvas.fillRect(state.buffer_ctx, rect, color);
      ();
    };

let handleMessage = (state, message) =>
  switch (message) {
  | Flush =>
    switch (state.status) {
    | CanFlush =>
      let _ =
        Canvas.fillRect(
          state.buffer_ctx,
          Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
          Canvas.Hex(0x36454f),
        );
      state.ops |> List.iter(paint(state));
      let _ =
        Canvas.fillRect(
          state.screen,
          Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
          Canvas.Hex(0x36454f),
        );
      let _ = Canvas.drawImage(state.buffer, state.screen, Point2D(0, 0));
      Process.Become({...state, ops: [], status: Flushed});
    | Flushed => Process.Become(state)
    }
  | Pipeline(ops) =>
    Process.Become({
      ...state,
      ops: List.concat([ops, state.ops]),
      status: CanFlush,
    })
  | _ => Process.Become(state)
  };

let loop: Process.f(t) =
  (env, state) => env.recv() >>| handleMessage(state) <|> Become(state);

let start = screenName => {
  let screen = DOM.elementById(screenName);
  let (w, h) = DOM.size(screen);
  let buffer = Canvas.create(w, h);
  let initialState = {
    /* TODO(@ostera): keep current flush color in state */
    status: CanFlush,
    buffer,
    buffer_ctx: Canvas.get2dContext(buffer),
    screen: Canvas.get2dContext(screen),
    ops: [],
  };
  let pid = spawn(loop, initialState) |> register(name);
  let _flusher = Game_Timer.start({target: pid, message: Flush});
  pid;
};
