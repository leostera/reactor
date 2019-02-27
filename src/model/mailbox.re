type t('m) = Queue.t('m);

let send = (mailbox, msg) => {
  Queue.push(msg, mailbox);
};

let recv = mailbox => {
  switch (Queue.pop(mailbox)) {
  | exception Queue.Empty => None
  | msg => Some(msg)
  };
};

let create = Queue.create;
