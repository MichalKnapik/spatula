#!/usr/bin/python

import sys

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        msg = """Usage: produceLinearGenericPipeline.py N fno
N: number of nodes, fno: formula number
formulae:
1 - #AF($var; (in1 and in2 and ... and outN/2 and ... outN) )
2 - #AG($var; #AF($var; (in1 and in2 and ... and inN) ));
2 - #EF($var; #AG($var; (in1 and out2 and in3 and ...) )); (N elements)
"""
        print(msg)
        sys.exit()

    nodesno = int(sys.argv[1])
    formno = int(sys.argv[2])

    normmodtext = """
module Node{0}:
  /* change this, don't touch the rest */
  nodeNo = {0};
  allNodes = {1};
  /* --------------------------------- */

  outnode = "state0";
  innode = "state1";
  bloom(outnode);
  bloom(innode);

  outmark = "out" + nodeNo;
  inmark = "in" + nodeNo;
  mark_with(outnode, outmark);
  mark_with(outnode, "initial");
  mark_with(innode, inmark);

  selfTrans = "act" + nodeNo;
  join_with(outnode, innode, selfTrans);

  if(nodeNo < allNodes) {{
    nextTrans = "act" + (nodeNo + 1);
    join_with(outnode, innode, nextTrans);
  }}
  if(nodeNo < (allNodes - 1)) {{
    nextnextTrans = "act" + (nodeNo + 2);
    join_with(outnode, innode, nextnextTrans);
  }}

  retact = "ret" + nodeNo;
  join_with(innode, outnode, retact);
"""

    print("semantics disjunctive;")

    for i in range(1, nodesno + 1):
        print(normmodtext.format(i,nodesno))

    outno = ""
    a = [outno + "out" + str(i) for i in range(1, nodesno + 1)]
    outs = " and ".join(a)

    inno = ""
    b = [inno + "in" + str(i) for i in range(1, nodesno + 1)]
    ins = " and ".join(b)

    mixes = " and ".join(a[:(int(nodesno/2))] + b[(int(nodesno/2)):])

    def dsel(n, li = a, lj = b):
        if n%2 == 0:
            return li[n]
        else:
            return lj[n]
    mixb = [dsel(i) for i in range(len(a))]
    mixbs = " and ".join(mixb)

    if(formno == 1): print("\nverify:\n#AF($var; ("+ mixes + ") );")
    if(formno == 2): print("\nverify:\n#AG($var; #AF($var;("+ ins + ") )) ;")
    if(formno == 3): print("\nverify:\n#EF($var; #AG($var; ("+ mixbs + ") ));")
 
