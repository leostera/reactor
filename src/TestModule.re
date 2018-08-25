open ReActor;

let f = (a, b) => {
  let c = a + b;
  Js.log({j|$a + $b = $c|j});
};

let run = () => Js.log(spawn());

run();

FFI_Runtime.defer(() => run(), 1000);
FFI_Runtime.defer(() => run(), 1000);
FFI_Runtime.defer(() => run(), 1000);
FFI_Runtime.defer(() => run(), 1000);
FFI_Runtime.defer(() => run(), 1000);
