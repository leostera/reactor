open Reactor;

let count = 1000;

let pid = {
  let of_triple = ((n, w, p)) => Pid.make(n, w, p);
  let pid_gen = QCheck.Gen.(triple(int, int, int) >|= of_triple);
  QCheck.make(pid_gen);
};

let pid_equality = {
  let prop = p => Pid.equal(p, p) == true;
  QCheck.Test.make(~count, ~name="a pid is equal to itself", pid, prop);
};

let next_pid_is_not_same_pid = {
  let prop = p => Pid.equal(p, p |> Pid.next) == false;
  QCheck.Test.make(~count, ~name="next pid is not same pid", pid, prop);
};

let view_is_consistent = {
  let prop = p => {
    let v = Pid.view(p);
    let p' =
      Pid.make(
        v.node_id |> Int32.to_int,
        v.worker_id |> Int32.to_int,
        v.process_id |> Int32.to_int,
      );
    let v' = Pid.view(p');
    v == v';
  };
  QCheck.Test.make(
    ~count,
    ~name="a pid is consistent with it's view",
    pid,
    prop,
  );
};

let suite =
  List.map(
    QCheck_alcotest.to_alcotest,
    [pid_equality, next_pid_is_not_same_pid, view_is_consistent],
  );

let () = Alcotest.run("Model.Pid", [("core", suite)]);
