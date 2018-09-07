open Option;
open ReActor;
open Game_FFI;

let name = "input_handler";

type Message.t +=
  | Event(Event.data)
  | Subscribe(Event.kind, Pid.t)
  | Unsubscribe(Event.kind, Pid.t);

type state = {
  node: DOM.node,
  handlers: Event.Map.t(Event.handler),
  subscribers: Event.Map.t(list(Pid.t)),
};

/** Find the subscribers for the given event name, and forward the event to them */
let fanout = (subs, event) => {
  Event.preventDefault(event);
  let kind = Event.kind(event);
  switch (Event.Map.find(kind, subs)) {
  | exception Not_found => ()
  | pids => pids |> List.iter(pid => send(pid, Event(Event.data(event))))
  };
};

let subscribe = (event_name, pid, subs) => {
  let pids =
    switch (Event.Map.find(event_name, subs)) {
    | exception Not_found => []
    | x => x
    };
  let pids' = [pid, ...pids];
  subs |> Event.Map.remove(event_name) |> Event.Map.add(event_name, pids');
};

let unsubscribe = (event_name, pid, subs) => {
  let pids =
    switch (Event.Map.find(event_name, subs)) {
    | exception Not_found => []
    | x => x
    };
  let pids' = pids |> List.filter(p => p == pid);
  subs |> Event.Map.remove(event_name) |> Event.Map.add(event_name, pids');
};

let setupHandlers = (event_name, node, handlers, subs) => {
  let handler' = fanout(subs);
  let handlers' =
    switch (Event.Map.find(event_name, handlers)) {
    | exception Not_found => handlers
    | handler =>
      node |> DOM.off(event_name, handler) |> ignore;
      handlers |> Event.Map.remove(event_name);
    };
  node |> DOM.on(event_name, handler') |> ignore;
  handlers' |> Event.Map.add(event_name, handler');
};

/**
  Update the handler for the newly subscribed/unsubscribed event and continue
 */
let continue = (event_name, {node, handlers}, subscribers') => {
  let handlers' = setupHandlers(event_name, node, handlers, subscribers');
  Process.Become({node, handlers: handlers', subscribers: subscribers'});
};

let handleMessage = state =>
  fun
  | Subscribe(event_name, pid) =>
    subscribe(event_name, pid, state.subscribers)
    |> continue(event_name, state)
  | Unsubscribe(event_name, pid) =>
    unsubscribe(event_name, pid, state.subscribers)
    |> continue(event_name, state)
  | _ => Process.Become(state);

let loop: Process.f(state) =
  (env, state) => env.recv() >>| handleMessage(state) <|> Become(state);

let start = () => {
  let initialState = {
    node: DOM.window,
    handlers: Event.Map.empty,
    subscribers: Event.Map.empty,
  };
  spawn(loop, initialState) |> register(name);
};
