#!/usr/bin/python

import sys

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        msg = """Usage: produceFTGC.py N fno
N: number of trains, fno: formula number
formulae:
1 - #AG($var; ( (not (Train1in and Train2in)) or ... )) (each pair)
    and #EF($var; Train1in) and ... (each train)
2 - #AG($var; (#EX($var; (not Train1in or Train1in))))
    with switches: appr1, in1, inF1, out1, outF1;"""

        print(msg)
        sys.exit()

    trainsno = int(sys.argv[1])
    formno = int(sys.argv[2])

    controllertext = """
module Controller:
  trainsNo = {0};
  faultyTrainNo = 1;
  
  /* correct behaviour */
  bloom("green");
  mark_with("green", "initial");
  bloom("red");
  ctr = 1;
  while(ctr <= trainsNo) {{
     outlabel = "out" + ctr;
     inlabel  = "in" + ctr;
     join_with("green", "red", inlabel);
     join_with("red", "green", outlabel);
     ctr = ctr + 1;
  }}

  /* faulty behaviour */
  inlabelF = "inF" + faultyTrainNo;
  outlabelF = "outF" + faultyTrainNo;
  join_with("green", "green", inlabelF);	
  join_with("red", "red", outlabelF);
"""

    traintext = """
module Train{0}:
  trainNo = {0};
  faultyTrainNo = 1;

  bloom("out");
  mark_with("out", "initial");
  bloom("approaching");
  bloom("in");

  outlabel = "out" + trainNo;
  inlabel  = "in" + trainNo;
  apprlabel  = "appr" + trainNo;
  join_with("in", "out", outlabel);
  join_with("out", "approaching", apprlabel);
  join_with("approaching", "in", inlabel);

  /* faulty behaviour */
  if(trainNo == faultyTrainNo) {{
    inlabelF = "inF" + faultyTrainNo;
    outlabelF = "outF" + faultyTrainNo;  	   
    join_with("in", "out", outlabelF);
    join_with("approaching", "in", inlabelF);
  }}

  /* label the nodes */
  outmark = "Train" + trainNo + "out";
  inmark = "Train" + trainNo + "in";
  apprmark = "Train" + trainNo + "approaching";
  mark_with("out", outmark);
  mark_with("in", inmark);
  mark_with("in", apprmark);
"""

    print("semantics disjunctive;")
    print(controllertext.format(trainsno))
    for i in range(1, trainsno + 1):
        print(traintext.format(i))

    print("\nverify:")
    
    forms = []
    mixtrin = ["(not (Train"+str(i)+"in and " + "Train"+str(j)+"in))" for i in range(1, trainsno + 1) for j in range(1, trainsno + 1) if i<j]
    formp1 = "#AG($var; (" + "\n or ".join(mixtrin) + "))"
    for i in range(1, trainsno + 1):
        formp1 = formp1 + "\nand #EF($var; Train"+str(i)+"in)"
    formp1 = formp1 + ";"
    forms.append(formp1)
    formp2 = "#AG($var; (#EX($var; (not Train1in or Train1in))));"
    formp2 = formp2 + "\nswitches: appr1,in1, inF1, out1, outF1;"
    forms.append(formp2)

    print(forms[formno-1])

