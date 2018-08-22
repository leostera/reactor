/***

 Common Window FFIs

 **/

[@bs.val]
external __unsafe_hardwareConcurrency: int =
  "self.navigator.hardwareConcurrency";

let hardwareConcurrency = __unsafe_hardwareConcurrency;
