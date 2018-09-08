open Option;
open ReActor;
open Game_FFI;

let name = "renderer";

type ops =
  | Fill(Canvas.color)
  | DrawText(string, Canvas.point, Canvas.color)
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
  | DrawText(text, origin, color) => {
      let _ = Canvas.fillText(state.buffer_ctx, text, origin, color);
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
          state.screen,
          Canvas.Rect(0, 0, Viewport.width(), Viewport.height()),
          Canvas.Hex(0x36454f),
        );
      let _ = Canvas.drawImage(state.buffer, state.screen, Point2D(0, 0));
      Process.Become({...state, status: Flushed});
    | Flushed => Process.Become(state)
    }
  | Pipeline(ops) =>
    ops |> List.iter(paint(state));
    Process.Become({...state, status: CanFlush});
  | _ => Process.Become(state)
  };

let loop: Process.f(t) =
  (env, state) => env.recv() >>| handleMessage(state) <|> Become(state);

let start = () => {
  /* TODO(@ostera): allow for Size(1000, 1000) */
  let buffer = Canvas.create();
  let initialState = {
    /* TODO(@ostera): keep current flush color in state */
    status: CanFlush,
    buffer,
    buffer_ctx: buffer |> Canvas.get2dContext,
    screen: DOM.elementById("game") |> Canvas.get2dContext,
  };
  let pid = spawn(loop, initialState) |> register(name);
  let _flusher = Game_Timer.(start({target: pid, message: Flush, time: 16}));
  pid;
};
