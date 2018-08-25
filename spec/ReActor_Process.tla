-------------------------- MODULE ReActor_Process --------------------------
EXTENDS Naturals, Sequences
CONSTANTS Message
VARIABLES pid, status, ctl, mailbox
----------------------------------------------------------------------------

TypeInvariant == /\ pid \in [ node |-> Nat, scheduler |-> Nat, proc |-> Nat ]
                 /\ status \in { "alive", "dead" }
                 /\ mailbox \in Seq(Message)

Init == /\ pid \in << Nat, Nat, Nat >>
        /\ status = "alive"

Deliver(m) == /\ status = "alive"
              /\ mailbox' = Append(mailbox, m)

Recv == /\ status = "alive"
        /\ mailbox' = Tail(mailbox)

Exit == /\ status = "alive"
        /\ status' = "dead"

Next == /\ \E m \in Message : Deliver(m)
        /\ Recv
        /\ Exit

Spec == Init /\ [][Next]_<<status, mailbox>>

----------------------------------------------------------------------------
THEOREM Spec => []TypeInvariant
=============================================================================
\* Modification History
\* Last modified Sat Aug 25 17:11:53 CEST 2018 by ostera
\* Created Fri Aug 24 18:49:14 CEST 2018 by ostera
