open Option;
open ReActor;
open Game_FFI;

let name = "input_handler";

type Message.t +=
  | Event(Event.event)
  | Subscribe(Event.kind, Pid.t)
  | Unsubscribe(Event.kind, Pid.t);

type state = {
  node: DOM.node,
  handlers: Event.Map.t(Event.handler),
  subscribers: Event.Map.t(list(Pid.t)),
};

/** Find the subscribers for the given event name, and forward the event to them */
let fanout = (subs, event) =>
  switch (subs |> Event.Map.find(Event.kind(event))) {
  | exception Not_found => ()
  | pids => pids |> List.iter(pid => send(pid, Event(event)))
  };

let subscribe = (event_name, pid, subs) => {
  let pids' = [pid, ...Event.Map.find(event_name, subs)];
  let subs' =
    subs |> Event.Map.remove(event_name) |> Event.Map.add(event_name, pids');
  subs';
};

let unsubscribe = (event_name, pid, subs) => {
  let pids = subs |> Event.Map.find(event_name);
  let pids' = pids |> List.filter(p => p == pid);
  let subs' =
    subs |> Event.Map.remove(event_name) |> Event.Map.add(event_name, pids');
  subs';
};

let setupHandlers = (event_name, node, handlers, subs) => {
  let handler' = fanout(subs);
  let handlers' =
    switch (handlers |> Event.Map.find(event_name)) {
    | handler =>
      node |> DOM.off(event_name, handler) |> ignore;
      handlers |> Event.Map.remove(event_name);
    | exception Not_found => handlers
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

let setup: Process.f(state) =
  (env, {node, handlers, subscribers}) => {
    let handlers' = setupHandlers(Event.Click, node, handlers, subscribers);
    loop(env, {node, subscribers, handlers: handlers'});
  };

let start = nodeId => {
  let initialState = {
    node: DOM.elementById(nodeId),
    handlers: Event.Map.empty,
    subscribers: Event.Map.empty,
  };
  spawn(setup, initialState) |> register(name);
};
