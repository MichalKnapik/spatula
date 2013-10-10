GPP     	= g++
CC		= gcc
CDIR 	     	= ../cudd
CUDD_INCLUDE 	= $(CDIR)/include/ 
GPPOPTS		= -g -Wall -lm -I../ -I../model -I.
CCOPTS  	= -g -Wall 
BIFLSRC 	= ltsNet.l ltsNet.y
CPAROBJ  	= graph.o vectr.o tools.o ast.o ltsNet.tab.o lex.yy.o
CPARSRC 	= graph.c vectr.c tools.c ast.c

wrapper: ltsNetCppWrapper.cc $(CPAROBJ)
	$(GPP) -c $(GPPOPTS) -I$(CUDD_INCLUDE) ltsNetCppWrapper.cc

$(CPAROBJ): $(BIFLSRC) $(CPARSRC)
	bison -d ltsNet.y
	flex ltsNet.l
	$(CC) -c $(CCOPTS) ltsNet.tab.c lex.yy.c $(CPARSRC) -lfl

clean:
	rm -rf *~ *.yy.c *.o ltsNet *.tab.*
