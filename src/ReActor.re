let main = ReActor_Scheduler.make(~checkupFreq=500);

Js.log("Created main scheduler...");

main |> ReActor_Scheduler.loop;

/* spawn(BsModule("ReActor"), "f", [1, 2]); */

let f = (a, b) => {
  let c = a + b;
  Js.log({j|$a + $b = $c|j});
};
