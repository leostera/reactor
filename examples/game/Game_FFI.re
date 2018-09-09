module Viewport = {
  [@bs.deriving abstract]
  type t = {
    height: int,
    width: int,
  };

  [@bs.val] [@bs.scope "window"]
  external __unsafe_visualViewport: t = "visualViewport";

  let width = () => __unsafe_visualViewport |> widthGet;

  let height = () => __unsafe_visualViewport |> heightGet;
};

module Event = {
  /** Abstract event type */
  [@bs.deriving abstract]
  type event = {
    [@bs.as "type"]
    _type: string,
    pageX: int,
    pageY: int,
    [@bs.as "key"]
    keyName: string,
    keyCode: int,
  };

  /** FFI to preventDefault */
  [@bs.send]
  external __unsafe_preventDefault: event => unit = "preventDefault";
  let preventDefault = __unsafe_preventDefault;

  type handler = event => unit;

  type data =
    | NoData
    | KeyDownData(string, int)
    | ClickData(int, int)
    | MouseMoveData(int, int)
    | ResizeData(int, int);

  type kind =
    | Unknown
    | KeyDown
    | Resize
    | Click
    | MouseMove;

  let toString =
    fun
    | Unknown => "unknown"
    | KeyDown => "keydown"
    | Resize => "resize"
    | Click => "click"
    | MouseMove => "mousemove";

  let fromString =
    fun
    | "click" => Click
    | "mousemove" => MouseMove
    | "resize" => Resize
    | "keydown" => KeyDown
    | _ => Unknown;

  module Map =
    Map.Make({
      type t = kind;
      let compare = (a, b) => String.compare(toString(a), toString(b));
    });

  let kind: event => kind = e => _typeGet(e) |> fromString;

  let data = event =>
    switch (kind(event)) {
    | Resize => ResizeData(Viewport.width(), Viewport.height())
    | Click => ClickData(pageXGet(event), pageYGet(event))
    | MouseMove => MouseMoveData(pageXGet(event), pageYGet(event))
    | KeyDown => KeyDownData(keyNameGet(event), keyCodeGet(event))
    | Unknown => NoData
    };
};

module DOM = {
  /** Abstract type for getting an element */
  type node;

  [@bs.val] external __unsafe_document: node = "document";
  let document = __unsafe_document;

  [@bs.val] external __unsafe_window: node = "window";
  let window = __unsafe_window;

  /** FFI to create DOM element */
  [@bs.val] [@bs.scope "document"]
  external __unsafe_createElement: string => node = "createElement";
  let createElement = __unsafe_createElement;

  /** FFI to a DOM element */
  [@bs.val] [@bs.scope "document"]
  external __unsafe_getElementById: string => node = "getElementById";
  let elementById = __unsafe_getElementById;

  /** FFI to subscribe to events on a particular DOM node */
  [@bs.send]
  external __unsafe_addEventListener: (node, string, Event.handler) => unit =
    "addEventListener";
  let on = (event, handler, node) => {
    __unsafe_addEventListener(node, Event.toString(event), handler);
    node;
  };

  /** FFI to unsubscribe to events on a particular DOM node */
  [@bs.send]
  external __unsafe_removeEventListener: (node, string, Event.handler) => unit =
    "removeEventListener";
  let off = (event, handler, node) => {
    __unsafe_removeEventListener(node, Event.toString(event), handler);
    node;
  };

  [@bs.get] external __unsafe_getWidth: node => int = "width";
  [@bs.get] external __unsafe_getHeight: node => int = "height";
  let size = node => (node |> __unsafe_getWidth, node |> __unsafe_getHeight);
};

module Canvas = {
  /** Abstract type for a canvas */
  type canvas;

  /** FFI to a Canvas */
  [@bs.val] [@bs.scope "document"]
  external __unsafe_createCanvas: ([@bs.as "canvas"] _, unit) => canvas =
    "createElement";
  [@bs.set] external __unsafe_width: (canvas, int) => unit = "width";
  [@bs.set] external __unsafe_height: (canvas, int) => unit = "height";
  let create = (w, h) => {
    let canvas = __unsafe_createCanvas();
    __unsafe_width(canvas, w);
    __unsafe_height(canvas, h);
    canvas;
  };

  /** Abstract type for a canvas context */
  type context;

  /** FFI to a Canvas Context */
  [@bs.send]
  external __unsafe_getContext: ('t, string) => context = "getContext";
  let get2dContext = node => __unsafe_getContext(node, "2d");

  /** Point type */
  type point =
    | Point2D(int, int);

  /** Rect Type */
  type shape =
    | Rect(int, int, int, int);

  /** Clockwise-ity */
  type angular_direction =
    | Clockwise
    | CounterClockwise;

  /** Color Type */
  type color =
    | Hex(int)
    | RGB(int, int, int)
    | RGBA(int, int, int, float);

  let colorToString =
    fun
    | Hex(h) => Printf.sprintf("%X", h) |> (hex => {j|#$hex|j})
    | RGB(r, g, b) => {j|rgb($r, $g, $b)|j}
    | RGBA(r, g, b, a) => {j|rgb($r, $g, $b, $a)|j};

  /** FFI to fill in a shape with a color */;
  [@bs.set]
  external __unsafe_fillStyle: (context, string) => unit = "fillStyle";
  let fillStyle = (canvas, color) =>
    __unsafe_fillStyle(canvas, color |> colorToString);

  /** FFI to fill in a shape with a color */;
  [@bs.send]
  external __unsafe_fillRect: (context, int, int, int, int) => unit =
    "fillRect";

  let fillRect = (canvas, Rect(x, y, w, h), color) => {
    fillStyle(canvas, color);
    __unsafe_fillRect(canvas, x, y, w, h);
    canvas;
  };

  /** FFI to draw text with a color */;
  [@bs.set] external __unsafe_font: (context, string) => unit = "font";
  [@bs.send]
  external __unsafe_fillText: (context, string, int, int) => unit = "fillText";
  let fillText = (canvas, text, font, Point2D(x, y), color) => {
    fillStyle(canvas, color);
    __unsafe_font(canvas, font);
    __unsafe_fillText(canvas, text, x, y);
    canvas;
  };

  [@bs.send]
  external __unsafe_drawImage: (context, canvas, int, int) => unit =
    "drawImage";
  let drawImage = (canvas, context, Point2D(x, y)) =>
    __unsafe_drawImage(context, canvas, x, y);

  [@bs.send] external __unsafe_beginPath: context => unit = "beginPath";
  let beginPath = __unsafe_beginPath;

  [@bs.send] external __unsafe_fill: context => unit = "fill";
  let fill = __unsafe_fill;

  [@bs.send]
  external __unsafe_arc: (context, int, int, int, float, float, bool) => unit =
    "arc";
  let arc = (context, Point2D(x, y), radius, beginAngle, endAngle, direction) => {
    let direction' =
      switch (direction) {
      | Clockwise => false
      | CounterClockwise => true
      };
    __unsafe_arc(context, x, y, radius, beginAngle, endAngle, direction');
  };
};
