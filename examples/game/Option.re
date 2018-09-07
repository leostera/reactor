let (>>|): (option('a), 'a => 'b) => option('b) =
  (o, f) =>
    switch (o) {
    | Some(x) => Some(f(x))
    | None => None
    };

let (<|>): (option('a), 'a) => 'a =
  (a, b) =>
    switch (a) {
    | Some(x) => x
    | None => b
    };
