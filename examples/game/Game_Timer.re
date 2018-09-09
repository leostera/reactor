open ReActor;

type state = {
  target: Pid.t,
  message: Message.t,
};

let loop: Process.f(state) =
  (_env, state) => {
    state.target <- state.message;
    OnAnimationFrame(state);
  };

let start = spawn(loop);
