module Event = {
  /** Abstract event type */
  [@bs.deriving abstract]
  type event = {
    [@bs.as "type"]
    _type: string,
  };

  type handler = event => unit;

  type kind =
    | Click
    | MouseMove;

  let kind: event => kind = event => Click;

  let toString =
    fun
    | Click => "click"
    | MouseMove => "mousemove";

  module Map =
    Map.Make({
      type t = kind;
      let compare = (a, b) => String.compare(toString(a), toString(b));
    });
};

module DOM = {
  /** Abstract type for getting an element */
  type node;

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
};

module Canvas = {
  /** Abstract type for a canvas context */
  type context;
  /** FFI to a Canvas Context */
  [@bs.send]
  external __unsafe_getContext: (DOM.node, string) => context = "getContext";
  let get2dContext = node => __unsafe_getContext(node, "2d");

  /** Rect Type */
  type shape =
    | Rect(int, int, int, int);

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
  [@bs.send]
  external __unsafe_fillRect: (context, int, int, int, int) => unit =
    "fillRect";

  let fillRect = (canvas, Rect(x, y, w, h), color) => {
    __unsafe_fillStyle(canvas, color |> colorToString);
    __unsafe_fillRect(canvas, x, y, w, h);
    canvas;
  };
};
