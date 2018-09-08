// Generated by BUCKLESCRIPT VERSION 4.0.5, PLEASE EDIT WITH CARE

import * as $$Array from "../../../node_modules/bs-platform/lib/es6/array.js";
import * as Block from "../../../node_modules/bs-platform/lib/es6/block.js";
import * as Curry from "../../../node_modules/bs-platform/lib/es6/curry.js";
import * as ReActor from "../src/ReActor.bs.js";
import * as Caml_array from "../../../node_modules/bs-platform/lib/es6/caml_array.js";
import * as Js_primitive from "../../../node_modules/bs-platform/lib/es6/js_primitive.js";
import * as ReActor_Utils from "../src/ReActor_Utils.bs.js";
import * as Caml_exceptions from "../../../node_modules/bs-platform/lib/es6/caml_exceptions.js";

var __name = "logger";

var Log = Caml_exceptions.create("E4_MoreCollaboratingProcesses.Logger.Log");

function logger_f(env, prefix) {
  var match = Curry._1(env[/* recv */1], /* () */0);
  if (match !== undefined) {
    var m = Js_primitive.valFromOption(match);
    if (m[0] === Log) {
      console.log("" + (String(prefix) + ("" + (String(m[1]) + ""))));
      return /* Become */Block.__(0, [prefix]);
    } else {
      return /* Become */Block.__(0, [prefix]);
    }
  } else {
    return /* Become */Block.__(0, [prefix]);
  }
}

var logger = ReActor.register(__name, ReActor.spawn(logger_f, "Default => "));

var Logger = /* module */[
  /* __name */__name,
  /* Log */Log,
  /* logger_f */logger_f,
  /* logger */logger
];

var Diff = Caml_exceptions.create("E4_MoreCollaboratingProcesses.Differ.Diff");

function f(env, config) {
  var match = Curry._1(env[/* recv */1], /* () */0);
  if (match !== undefined) {
    var m = Js_primitive.valFromOption(match);
    if (m[0] === Diff) {
      var delta = ReActor_Utils.Performance[/* now */0](/* () */0) - m[1] | 0;
      ReActor.send(config[/* send_to */0], Curry._1(config[/* wrap */1], delta));
      return /* Become */Block.__(0, [config]);
    } else {
      return /* Become */Block.__(0, [config]);
    }
  } else {
    return /* Become */Block.__(0, [config]);
  }
}

function start(param) {
  return ReActor.spawn(f, param);
}

var Differ = /* module */[
  /* Diff */Diff,
  /* f */f,
  /* start */start
];

function clock_f(_, config) {
  ReActor.send(config[/* send_to */1], Curry._1(config[/* wrap */2], ReActor_Utils.Performance[/* now */0](/* () */0)));
  return /* Become */Block.__(0, [config]);
}

function start$1(param) {
  return ReActor.spawn(clock_f, param);
}

var Clock = /* module */[
  /* clock_f */clock_f,
  /* start */start$1
];

var match = ReActor.where_is("logger");

if (match !== undefined) {
  var pid = match;
  $$Array.iter((function () {
          var differ = ReActor.spawn(f, /* record */[
                /* send_to */pid,
                /* wrap */(function (x) {
                    return [
                            Log,
                            x
                          ];
                  })
              ]);
          ReActor.spawn(clock_f, /* record */[
                /* delay */100,
                /* send_to */differ,
                /* wrap */(function (x) {
                    return [
                            Diff,
                            x
                          ];
                  })
              ]);
          return /* () */0;
        }), Caml_array.caml_make_vect(10, 0));
} else {
  console.log("Failed to start logger.");
}

export {
  Logger ,
  Differ ,
  Clock ,
  
}
/* logger Not a pure module */