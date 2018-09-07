open ReActor;

type state = {
  target: Pid.t,
  message: Message.t,
  time: int,
};

let loop: Process.f(state) =
  (_env, state) => {
    send(state.target, state.message);
    OnAnimationFrame(state);
  };

let start = spawn(loop);
