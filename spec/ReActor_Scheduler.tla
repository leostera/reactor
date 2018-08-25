------------------------- MODULE ReActor_Scheduler -------------------------
EXTENDS Naturals, Sequences
CONSTANTS Message, Process, MFA, Response

VARIABLES sid, ctl, processes, process_count, last_pid

allvars == <<sid, ctl, processes, process_count, last_pid>>

----------------------------------------------------------------------------

TypeInvariant == /\ sid \in [ node |-> Nat, scheduler |-> Nat ]
                 /\ processes \in Seq(Process)

Init == /\ sid \in << Nat, Nat >>
        /\ processes = <<>>
        /\ process_count = 0

Spawn(r, f) == /\ process_count' = process_count + 1
               /\ last_pid'  = Append(sid, process_count')
               /\ processes' = Append(processes, << last_pid, f >>)
               /\ r' = last_pid'
               /\ UNCHANGED <<sid>>

Info(r) == /\ r' = [ process_count |-> process_count ]
           /\ UNCHANGED <<sid, processes, process_count>>

GC == /\ LET isAlive(x) == x.status = "alive" IN
         processes' = SelectSeq(processes, isAlive)
      /\ process_count' = Len(processes')
      /\ UNCHANGED <<sid>>

Next == /\ \E f \in MFA, res \in Response : Spawn(res, f)
        /\ \E res \in Response : Info(res)
        /\ GC

Spec == Init /\ [][Next]_allvars

----------------------------------------------------------------------------
THEOREM Spec => []TypeInvariant
=============================================================================
\* Modification History
\* Last modified Sat Aug 25 17:57:24 CEST 2018 by ostera
\* Created Sat Aug 25 16:46:06 CEST 2018 by ostera
