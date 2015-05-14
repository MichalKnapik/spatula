#!/usr/bin/env python

import sys

def getProcess(i):
    print("module Process{}:".format(str(i)))
    print("  procnumber = {};".format(str(i)))
    print("""
  idlen = "idle" + procnumber;
  tryingn = "trying" + procnumber;
  waitingn = "waiting" + procnumber;
  criticaln = "critical" + procnumber;

  bloom(idlen);
  bloom(tryingn);
  bloom(waitingn);
  bloom(criticaln);

  mark_with(idlen, "initial");

  critprop = "critical" + procnumber;
  mark_with(criticaln, critprop);
  
  startnl = "Start" + procnumber;
  join_with(idlen, tryingn, startnl);

  setxl = "SetX" + procnumber;
  join_with(tryingn,  waitingn, setxl);

  enterl = "Enter" + procnumber;
  join_with(waitingn, criticaln, enterl);

  retxl = "SetX0" + procnumber;
  join_with(criticaln,  idlen, retxl);
""")


def getVariable(i):

  print("module Variable:")
  print("processes = {};".format(str(i)))
  print("""  
  i = 0;
  while( i <= processes ) {
     valn = "Val" + i;
     bloom(valn);
     i = i+1;
  } 

  mark_with("Val0", "initial");

  i = 0;

  while( i <= processes ) {

       outv = "Val" + i;

       j = 0;    
       while( j <= processes ) {
         inv = "Val" + j;

         if( i == j ) {
	    if( i == 0 ) {
	     
	     k = 1;
	     while(k <= processes) {
	        startnl = "Start" + k;
	        join_with(outv, inv, startnl);
		k = k + 1;
		}
            } else {
	        setxl = "SetX" + i;	
                enterl = "Enter" + i; 
	        join_with(outv, inv, setxl);	      
	        join_with(outv, inv, enterl);	      
	    }   

	 } else {
		    if( j == 0 ) {
		      retxl = "SetX0" + i;
		      join_with(outv, inv, retxl);	      
		    } else {
		      setxl = "SetX" + j;
		      join_with(outv, inv, setxl);	      
		    }
         }


         j = j + 1;
       }

     i = i + 1;
  }
""")

def getEF(n):
    print("verify:\n#EF($var; (")
    print(" and ".join(["critical{}".format(str(i)) for i in range(1,n+1)]))
    print("));")

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        print("Usage: MutexGen.py componentNo criticalNo")
        sys.exit()
    
    compNo = int(sys.argv[1])
    critNo = int(sys.argv[2])

    print("semantics disjunctive;")
    
    for i in range(1,compNo + 1):
        getProcess(i)

    getVariable(compNo)
    getEF(critNo)
