type t('a) = Queue.t('a);

let create = Queue.create;

let queue = (q, el) => {
  Queue.add(el, q);
  q;
};

let next = q => {
  switch (Queue.pop(q)) {
  | exception _ => None
  | el => Some(el)
  };
};

let to_seq = Queue.to_seq;

let length = Queue.length;

let clear = q => {
  Queue.clear(q);
  q;
};
