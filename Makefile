GPP	     = g++
CDIR 	     = cudd
CUDD_INCLUDE = $(CDIR)/include/ 
CUDD_LIBS    = $(CDIR)/obj/cuddObj.o $(CDIR)/obj/libobj.a $(CDIR)/cudd/libcudd.a\
	       $(CDIR)/mtr/libmtr.a $(CDIR)/st/libst.a $(CDIR)/util/libutil.a\
	       $(CDIR)/epd/libepd.a
GPPOPTS      = -g -Wall -lm -Iparser -Imodel -I$(CUDD_INCLUDE) $(CUDD_LIBS)
CPAROBJ      = parser/graph.o parser/vectr.o parser/tools.o parser/ast.o\
	       parser/ltsNet.tab.o parser/lex.yy.o parser/ltsNetCppWrapper.o
MDLSRC       = model/model.cc model/modelTools.cc
CHECKRSC     = checker/checker.cc

all: bddcudd
	make -C parser
	$(GPP) $(GPPOPTS) $(CPAROBJ) $(MDLSRC) $(CHECKRSC) -o spatula

bddcudd:
	make -C cudd
	make -C cudd/obj

clean:
	make clean -C parser
	rm -rf *.o

cleancudd:
	make clean -C cudd
	make clean -C cudd/obj

