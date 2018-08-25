type pid = (int, int, int);

let pid = (node, proc_count, noise) => (node, proc_count, noise);

type m =
  | BsModule(string);

type mfa('a) = (m, string, 'a);

let mfa: (m, string, 'a => 'b, 'a) => mfa('a) = (m, f, _, a) => (m, f, a);

type process = {
  pid,
  f: unit => unit,
};
