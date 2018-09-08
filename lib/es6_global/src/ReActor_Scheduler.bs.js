// Generated by BUCKLESCRIPT VERSION 4.0.5, PLEASE EDIT WITH CARE

import * as List from "../../../node_modules/bs-platform/lib/es6/list.js";
import * as Caml_obj from "../../../node_modules/bs-platform/lib/es6/caml_obj.js";
import * as FFI_Runtime from "./FFI_Runtime.bs.js";
import * as ReActor_Utils from "./ReActor_Utils.bs.js";
import * as Caml_primitive from "../../../node_modules/bs-platform/lib/es6/caml_primitive.js";
import * as ReActor_Tracer from "./ReActor_Tracer.bs.js";
import * as ReActor_Process from "./ReActor_Process.bs.js";

function make(node_name) {
  return /* tuple */[
          node_name,
          ReActor_Utils.Random[/* shortId */3](/* () */0)
        ];
}

function toString(param) {
  return "<" + (String(param[0]) + ("." + (String(param[1]) + ">")));
}

var Sid = /* module */[
  /* make */make,
  /* toString */toString
];

function nextPid(param) {
  var match = param[/* id */0];
  return ReActor_Process.Pid[/* make */0](match[0], match[1], param[/* process_count */2] + 1 | 0);
}

function byProcessCount(a, b) {
  return Caml_primitive.caml_int_compare(a[0][/* process_count */2], b[0][/* process_count */2]);
}

function leastBusy(workers) {
  return List.hd(List.sort(byProcessCount, workers));
}

function pidToSid(param) {
  return /* tuple */[
          param[0],
          param[1]
        ];
}

function findById(i) {
  return (function (param) {
      return List.find((function (s) {
                    return Caml_obj.caml_equal(s[0][/* id */0], i);
                  }), param);
    });
}

function make$1(node_name) {
  return /* record */[
          /* id */make(node_name),
          /* processes : [] */0,
          /* process_count */0,
          /* tracer */undefined
        ];
}

function spawn(f, args, scheduler) {
  var pid = nextPid(scheduler[0]);
  var $$process = ReActor_Process.make(pid, f, args);
  var init = scheduler[0];
  var scheduler$prime_000 = /* id */init[/* id */0];
  var scheduler$prime_001 = /* processes : :: */[
    $$process,
    scheduler[0][/* processes */1]
  ];
  var scheduler$prime_002 = /* process_count */scheduler[0][/* process_count */2] + 1 | 0;
  var scheduler$prime_003 = /* tracer */init[/* tracer */3];
  var scheduler$prime = /* record */[
    scheduler$prime_000,
    scheduler$prime_001,
    scheduler$prime_002,
    scheduler$prime_003
  ];
  scheduler[0] = scheduler$prime;
  return pid;
}

function exit(pid, scheduler) {
  return ReActor_Process.markAsDead(List.find((function (p) {
                    return Caml_obj.caml_equal(p[/* pid */0], pid);
                  }), scheduler[0][/* processes */1]));
}

function send(pid, msg, scheduler) {
  var match = scheduler[0][/* tracer */3];
  if (match !== undefined) {
    ReActor_Tracer.trace(match, pid, msg);
  }
  return ReActor_Process.send(msg, List.find((function (p) {
                    return Caml_obj.caml_equal(p[/* pid */0], pid);
                  }), scheduler[0][/* processes */1]));
}

function trace(tracer, scheduler) {
  var timeout = tracer[/* timeout */2];
  console.log("[ReActor] Tracer setup for " + (String(timeout) + "  ms."));
  var init = scheduler[0];
  scheduler[0] = /* record */[
    /* id */init[/* id */0],
    /* processes */init[/* processes */1],
    /* process_count */init[/* process_count */2],
    /* tracer */tracer
  ];
  return FFI_Runtime.defer((function () {
                var init = scheduler[0];
                scheduler[0] = /* record */[
                  /* id */init[/* id */0],
                  /* processes */init[/* processes */1],
                  /* process_count */init[/* process_count */2],
                  /* tracer */undefined
                ];
                console.log("[ReActor] Tracer stopped after " + (String(timeout) + " ms."));
                return /* () */0;
              }), timeout);
}

export {
  Sid ,
  nextPid ,
  byProcessCount ,
  leastBusy ,
  pidToSid ,
  findById ,
  make$1 as make,
  spawn ,
  exit ,
  send ,
  trace ,
  
}
/* FFI_Runtime Not a pure module */