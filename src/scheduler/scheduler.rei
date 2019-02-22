module Policy: {
  type t;
  let default: unit => t;
};

let start: Policy.t => unit;

let send: (Model.Pid.t, Model.Message.t) => unit;
