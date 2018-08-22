type pid = (int, int, int);
type mfa('a) = (string, string, 'a);

type instruction =
  | Info
  | Spawn;

type reply('a) =
  | WorkerStatus('a);

type process = {
  pid,
  f: unit => unit,
};
