let main =
  ReActor_Node.make(
    ~checkupFreq=500,
    ~maxConcurrency=FFI_Window.hardwareConcurrency - 1,
  )
  |> ReActor_Node.run;

Js.log("Created main scheduler...");

let spawn = ReActor_Node.spawn(main);
