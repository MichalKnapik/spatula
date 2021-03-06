#!/usr/bin/python

import sys

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        msg = """Usage: produceFTGC.py N fno
N: number of trains, fno: formula number
formulae:
1 - #AG($var; ( not ( (Train1in and Train2in) or ... )) (each pair)
    and #EF($var; Train1in) and ... (each train)
2 - #EGom($varx; (#EF($vary; (Train1in or .. or TrainNin))));
3 - #EGom($var; (#EF($var; (Train1in or .. or TrainNin))));
4 - #EF($var;(#AG($var; ((not Train1in) and ... and (not TrainNin))));
"""

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
  mark_with("green", "Green");
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

    inacts = ["in"+str(i) for i in xrange(1,trainsno+1)]
    outacts = ["out"+str(i) for i in xrange(1,trainsno+1)]
    inactsF = ["inF1"]
    outactsF = ["outF1"]
    appracts = ["appr"+str(i) for i in xrange(1,trainsno+1)]
    
    forms = []
    switches = []

    mixtrin = ["(Train"+str(i)+"in and " + "Train"+str(j)+"in)" for i in range(1, trainsno + 1) for j in range(1, trainsno + 1) if i<j]

    formp1 = "#AG($var; ( not (" + "\n or ".join(mixtrin) + ")))"
    for i in range(1, trainsno + 1):
        formp1 = formp1 + "\nand #EF($var; Train"+str(i)+"in)"
    formp1 = formp1 + ";"
    forms.append(formp1)
    switches.append("")

    formp2 = "#EGom($varx;(#EF($vary; ( Train1in and Train2in))));\n"
    forms.append(formp2)
    switches.append("")


    formp3 = "#EGom($var;(#EF($var; ( Train1in and Train2in))));\n"
    forms.append(formp3)
    switches.append("")

    formp4 = "#EF($varx;(#AG($vary; (" + " and ".join(["(not Train"+str(i)+"in)" for i in range(1, trainsno + 1)]) + " and Green))));\n"
    forms.append(formp4)
    switches.append("")

    print(forms[formno-1])
    if len(switches[formno-1]) > 0:
        print("switches:")
        print(",\n".join(switches[formno-1]))
        print(";")

