/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the docs/COPYING.txt file.
*
*/

#include "model.h"
#include "ltsNetCppWrapper.h"
#include "modelTools.h"
#include "cuddObj.hh"
#include "checker.h"
#include <unistd.h>
#include <sys/time.h>
#include <cmath>

/* from ast.c */
extern formTree* formula;
extern vectr* limits;
extern int semantics;
extern Cudd manager;

Checker::Checker(Network& networkv, Cudd& managerv) : net(networkv), manager(managerv) {
    };

BDD Checker::synthesise(formTree* formula) {  
    //check booleans
    if(formula->path_selector == NONEPARA) {
        if(formula->opertr == MNONE) { //proposition
	    bool semanticsSel = (semantics == 1); //quick fix, semantics selection
            return net.getStatesMarkedWith((string) formula->varName, semanticsSel) * net.statespace * net.allVarSel;
            }
        else if(formula->opertr == BNOT) { //negation
            BDD right = synthesise(formula->right);
            return (!right) * net.statespace * net.allVarSel;
            }
        else if(formula->opertr == BAND) { //conjunction
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return left * right * net.statespace * net.allVarSel;
            }
        else if(formula->opertr == BOR) { //disjunction
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return (left + right) * net.statespace * net.allVarSel;
            }
        }

    //check nonparametrics
    if(formula->path_selector == EXISTS) {
        if(formula->opertr == MNEXT) { //EX
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_EX(ss, right) * net.allVarSel;
            }
        if(formula->opertr == MFUTURE) { //EF
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_EF(ss, right) * net.allVarSel;
            }
        else if(formula->opertr == MGLOBALLYOM) { //EGom
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_EGom(ss, right) * net.allVarSel;
            }
        else if(formula->opertr == MGLOBALLY) { //EG
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right) * net.allVarSel;
            return check_EG(ss, right);
            }
        else if(formula->opertr == MUNTIL) { //EU
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return check_EU(ss, left, right) * net.allVarSel;
            }
        }

    if(formula->path_selector == ALWAYS) {
        if(formula->opertr == MNEXT) { //AX
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_AX(ss, right) * net.allVarSel;
            }
        if(formula->opertr == MFUTURE) { //AF
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_AF(ss, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLY) { //AG
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_AG(ss, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLYOM) { //AGom
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD right = synthesise(formula->right);
            return check_AGom(ss, right) * net.allVarSel;
            }
        if(formula->opertr == MUNTIL) { //EU
            set<string> ss = stvectr2stringSet(formula->elements);
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return check_AU(ss, left, right) * net.allVarSel;
            }
        }

    //check parametrics
    if(formula->path_selector == EXISTSPARA) {
        if(formula->opertr == MNEXT) { //EX
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_EX(var, right) * net.allVarSel;
            }
        if(formula->opertr == MFUTURE) { //EF
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_EF(var, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLY) { //EG
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_EG(var, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLYOM) { //EGom
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_EGom(var, right) * net.allVarSel;
            }
        if(formula->opertr == MUNTIL) { //EU
            string var = formula->varName;
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return check_EU(var, left, right) * net.allVarSel;
            }
        }

    if(formula->path_selector == ALWAYSPARA) {
        if(formula->opertr == MNEXT) { //AX
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_AX(var, right) * net.allVarSel;
            }
        if(formula->opertr == MFUTURE) { //AF
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_AF(var, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLY) { //AG
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_AG(var, right) * net.allVarSel;
            }
        if(formula->opertr == MGLOBALLYOM) { //AGom
            string var = formula->varName;
            BDD right = synthesise(formula->right);
            return check_AGom(var, right) * net.allVarSel;
            }
        if(formula->opertr == MUNTIL) { //AU
            string var = formula->varName;
            BDD left = synthesise(formula->left);
            BDD right = synthesise(formula->right);
            return check_AU(var, left, right) * net.allVarSel;
            }
        }

    cerr << "Panic: internal error in synthesise. Check source." << endl;
    exit(1);
    }

// Nonparametric modalities

BDD Checker::check_EX(set<string> actions, BDD pstates) {
    BDD swpstates = pstates.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
    BDD result = net.getLabeledTransitions(actions) * swpstates;
    result = result.ExistAbstract(net.globalStatesPrimedCube);

    return result * net.statespace;
    }


BDD Checker::check_AX(set<string> actions, BDD pstates) {
    /* s |= AX_{actions} phi iff ( s |= neg EX_{actions} neg phi) */
    BDD negEXneg = (!check_EX(actions, (!pstates * net.statespace)));

    return negEXneg * net.statespace;
    }


BDD Checker::check_EF(set<string> actions, BDD pstates) {
    BDD locrel = net.getLabeledTransitions(actions);
    BDD result = pstates.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
    BDD previous = manager.bddZero();

    while(result != previous) {
        previous = result;
        BDD resultpreimage = (locrel * result).ExistAbstract(net.globalStatesPrimedCube);
        resultpreimage = resultpreimage.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = previous + resultpreimage;
        }

    return result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed) * net.statespace;
    }


BDD Checker::check_AF(set<string> actions, BDD pstates) {
    /* s |= AF_{actions} phi iff s |= neg EG_{actions} neg phi */
    return !check_EG(actions, (!pstates * net.statespace)) * net.statespace;
    }


BDD Checker::check_EGom(set<string> actions, BDD pstates) {
    BDD locrel = net.getLabeledTransitions(actions);
    BDD prev = manager.bddZero();
    BDD result = pstates;

    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (locrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = pstates * result;
        }

    return result * net.statespace;
    }


BDD Checker::check_AGom(set<string> actions, BDD pstates) {
    /* s |= AGom_{actions} phi iff s |= AG_{actions}(phi or AF_{actions} dead_{actions}) */
    BDD locrel = net.getLabeledTransitions(actions);
    BDD deads = deadOnes(locrel);

    return check_AG(actions, pstates + check_AF(actions, deads));
    }


BDD Checker::check_EG(set<string> actions, BDD pstates) {
    BDD locrel = net.getLabeledTransitions(actions);
    BDD prev = manager.bddZero();
    BDD result = pstates;

    /* deadGood is the set of states s.t. proposition defined by pstates holds there
       and they have no successor (deadlock) under considered actions set */
    BDD nonDead = locrel.ExistAbstract(net.globalStatesPrimedCube);
    BDD deadGood = (pstates * !nonDead) * net.statespace;

    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (locrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = pstates * result + deadGood;
        }

    return result * net.statespace;
    }


BDD Checker::check_AG(set<string> actions, BDD pstates) {
    /* s |= AG_{actions} phi iff s |= neg EF_{actions} neg phi */
    return !check_EF(actions, (!pstates * net.statespace)) * net.statespace;
    }


BDD Checker::check_EU(set<string> actions, BDD pstates, BDD qstates) {
    BDD locrel = net.getLabeledTransitions(actions);
    BDD prev = manager.bddZero();
    BDD result = qstates;

    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (locrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = qstates + pstates * result;
        }

    return result * net.statespace;
    }


BDD Checker::check_AU(set<string> actions, BDD pstates, BDD qstates) {
    /* s |= A phi U_{actions} psi iff
       s |= neg ( E[neg psi U_{actions} neg (psi or phi)] or EG_{actions} neg psi) */
    BDD negpstates = !pstates * net.statespace;
    BDD negqstates = !qstates * net.statespace;

    return !(check_EU(actions, negqstates, negqstates * negpstates) + check_EG(actions, negqstates)) * net.statespace;
    }

// Parametric modalities

BDD Checker::check_EX(string parVariable, BDD pstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);
    BDD swpstates = pstates.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);

    BDD result = (modrel * swpstates).ExistAbstract(net.globalStatesPrimedCube);

    return result * net.statespace;
    }


BDD Checker::check_AX(string parVariable, BDD pstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);

    BDD negEXneg = !check_EX(parVariable, !pstates * net.statespace);

    return negEXneg * net.statespace;
    }


BDD Checker::check_EF(string parVariable, BDD pstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);

    BDD result = pstates.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
    BDD previous = manager.bddZero();

    while(result != previous) {
        previous = result;
        BDD resultpreimage = (modrel * result).ExistAbstract(net.globalStatesPrimedCube);
        resultpreimage = resultpreimage.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = previous + resultpreimage;
        }

    return result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed) * net.statespace;
    }


BDD Checker::check_AF(string parVariable, BDD pstates) {
    /* s |= AF_{actions} phi iff s |= neg EG_{actions} neg phi */
    return !check_EG(parVariable, (!pstates * net.statespace)) * net.statespace;
    }


BDD Checker::check_EGom(string parVariable, BDD pstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);
    BDD prev = manager.bddZero();
    BDD result = pstates;

    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (modrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = pstates * result;
        }

    return result * net.statespace;
    }


BDD Checker::check_AGom(string parVariable, BDD pstates) {
    /* s |= AGom_{actions} phi iff s |= AG_{actions}(phi or AF_{actions} dead_{actions}) */
    BDD deads = parametricDeadOnes(&net.transition, &parVariable);
    
    return check_AG(parVariable, pstates + check_AF(parVariable, deads));
    }


BDD Checker::check_EG(string parVariable, BDD pstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);

    //get deadlock descriptors
    BDD allDead = parametricDeadOnes(&net.transition, &parVariable) * pstates;

    BDD prev = manager.bddZero();
    BDD result = pstates;
    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (modrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = pstates * result + allDead;
        }

    return result * net.statespace;
    }


BDD Checker::check_AG(string parVariable, BDD pstates) {
    return !check_EF(parVariable, (!pstates * net.statespace)) * net.statespace;
    }


BDD Checker::check_EU(string parVariable, BDD pstates, BDD qstates) {
    //substitute action vars in transition relation with ones for parVariable
    BDD modrel = net.changeActionVars(net.transition, parVariable);

    BDD prev = manager.bddZero();
    BDD result = qstates;

    while(result != prev) {
        prev = result;

        //preimage of result
        result = result.SwapVariables(net.allStateBDDVars, net.allStateBDDVarsPrimed);
        result = (modrel * result).ExistAbstract(net.globalStatesPrimedCube);
        result = qstates + pstates * result;
        }

    return result * net.statespace;
    }


BDD Checker::check_AU(string parVariable, BDD pstates, BDD qstates) {
    /* s |= A phi U_{actions} psi iff
       neg ( E[neg psi U_{actions} neg (psi or phi)] or EG_{actions} neg psi) */
    BDD negpstates = !pstates * net.statespace;
    BDD negqstates = !qstates * net.statespace;

    return !(check_EU(parVariable, negqstates, negqstates * negpstates) + check_EG(parVariable, negqstates)) * net.statespace;
    }


BDD Checker::deadOnes(BDD rel) {
    BDD origins = rel.ExistAbstract(net.globalStatesPrimedCube);

    return !origins * net.statespace;
    }


BDD Checker::parametricDeadOnes(BDD* paraRel, string* parVariable) {

    if(paraRel == NULL) paraRel = &net.transition;

    //(Note: This function should be optimised more)
    BDD actionsDisjunction, varActionsCube;

    if(parVariable != NULL) {
        actionsDisjunction = net.changeActionVars(!net.negatedActionsCube, *parVariable);
        varActionsCube = net.changeActionVars(net.actionsCube, *parVariable);
        }
    else {
        parVariable = &net.origVarName;
        actionsDisjunction = !net.negatedActionsCube;
        varActionsCube = net.actionsCube;
        }

    BDD stateOutVal = paraRel->ExistAbstract(net.globalStatesPrimedCube);
    stateOutVal = net.changeActionVars(stateOutVal, *parVariable);

    //reallyDead - states with no outgoing actions at all
    BDD nonDead = stateOutVal.ExistAbstract(varActionsCube);

    BDD reallyDead = (net.statespace * !nonDead);

    //sometimesDead - states augmented with valuations under which they're deadlocks
    //(the complement trick, see Param. Comp. Tree Logic with Knowledge, CS&P'11, App.)
    BDD sometimesDead = (!stateOutVal * nonDead);

    return (reallyDead + sometimesDead) * net.statespace * actionsDisjunction;
    }


void Checker::recNextCall(naivegroupsize sumVal, int depth, int bits, BDD& resbdd) {
  /* Note: all this can be optimised, but it's of low priority */
  static int totbits = bits * depth;
  if(depth > 0) {
    naivegroupsize shift = pow(2, bits);
    naivegroupsize nsumVal = sumVal * shift;
    for(naivegroupsize i = 1; i < shift; ++i) recNextCall(nsumVal + i, depth - 1, bits, resbdd);
  } else {
    int hlp = sumVal;
    vector<string> vvparvars(net.parVars.begin(), net.parVars.end());

    vector<string> vvactions; //limit the synthesised actions to allowed, if needed
    if(limits != NULL) 
      vvactions.insert(vvactions.begin(), net.allowedToRemoveActions.begin(), net.allowedToRemoveActions.end());
    else vvactions.insert(vvactions.begin(), net.actions.begin(), net.actions.end());

    int currvno = -1;
    int actNo = -1;
    map<string, vector<string> > valuation;
    for(int b = 0; b < totbits; ++b, ++actNo) {
      if(b % bits == 0) {
	++currvno; //vvparvars[currvno] is a current variable
	if(limits != NULL) { //copy constant part of valuations set, if needed
	  valuation[vvparvars[currvno]].insert(valuation[vvparvars[currvno]].end(), net.constactions.begin(), net.constactions.end());	  
	}
	actNo = 0;
      }
      if(hlp % 2 == 1) { //insert actions in valuation
	valuation[vvparvars[currvno]].push_back(vvactions[actNo]);
      } 
     hlp /= 2;
    }

    //valuation should be ready to process now
    formTree* substForm = substitute(net.arctlForm, valuation);
    BDD res = synthesise(substForm);
    BDD st = net.initGlobal * net.allVarSel;
    if(res * st == st) {
      resbdd += net.valuation2BDD(valuation);
    }
    freeFormTree(substForm);
  }

}


BDD Checker::naiveSynth(formTree* formula) {
  BDD result = manager.bddZero();

  //generate and check all valuations
  int bitNo = 0;
  if(limits != NULL) bitNo = net.allowedToRemoveActions.size();
  else bitNo = net.actions.size();
  int parNo = net.parVars.size();
  recNextCall(0, parNo, bitNo, result);

  return result;
}


void usage(string pname) {
    cout << "Usage: " << pname << " [-svhn] -f model " << endl;
    cout << "Run " << pname << " -h for help" << endl;
    }

void preamble() {
    cout << "----------------------------------------" << endl;
    cout << "Spatula: simple parametric temporal tool" << endl;
    cout << "Distributed under GNU GPL v2" << endl;
    cout << "(c) Michal Knapik, ICS PAS 2013" << endl;
    cout << "----------------------------------------" << endl;
    }

Cudd manager(0);
int main(int argc, char** argv) {
    preamble();

    int o = 0;
    bool verbose = false;
    bool simulate = false;
    bool fileGiven = false;
    bool all = false;
    bool naive = false;
    bool minimal = false;
    string modelFile;

    if(argc < 2) {
        usage(argv[0]);
        exit(1);
        }

    while ((o = getopt(argc, argv,"mansvhf:")) != -1) {
        switch(o) {
            case 's':
                simulate = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'a':
                all = true;
                break;
            case 'n':
                naive = true;
                break;
            case 'N':
                naive = true;
                break;
            case 'f':
                modelFile = optarg;
                fileGiven = true;
                break;
            case 'm':
                minimal = true;
                break;
            case 'h':
                cout << "Usage: " << argv[0] << " [-s][-v] -f model " << endl;
                cout << "-f model is a required link to model file" << endl;
                cout << "-a answers 'yes' to all questions" << endl;
                cout << "-v stands for 'verbose'; gives additional processing info" << endl;
                cout << "-n performs naive, iterative benchmark" << endl;
                cout << "-m finds minimal valuations for monotone formulae" << endl;
                cout << "-s runs interactive simulation" << endl;
                exit(0);
                break;
            default:
                usage(argv[0]);
                exit(1);
            }
        }
    if(!fileGiven) {
        usage(argv[0]);
        exit(1);
        }
    
    struct timeval timr;

    Network net;
    cout << "Reading and building the model" << endl;

    gettimeofday(&timr, NULL);
    double buildstart = timr.tv_sec + (timr.tv_usec/1000000.0);
    if(!net.buildNetwork(modelFile, verbose)) exit(1);
    gettimeofday(&timr, NULL);
    double buildend = timr.tv_sec + (timr.tv_usec/1000000.0);
    cout << "Found " << net.statespace.CountMinterm(net.bits) << " reachable states (" << buildend - buildstart << " sec.)" << endl;

    if(simulate) {
      if(limits != NULL) { 
	  cout << "Can't simulate with switches on! Remove them and start again, sorry about that." << endl;
	  exit(1);
        }
        cout << "Running simulation" << endl;
        net.simulate();
        }
    Checker c(net, manager);

    BDD res;
    cout << "Running the synthesis/verification, ";
    gettimeofday(&timr, NULL);
    double synstart = timr.tv_sec + (timr.tv_usec/1000000.0);
    if(!naive) { //parametric verification
      cout << "using the parametric engine" << endl;
      res = c.synthesise(net.arctlForm);
    } else { //naive verification
      cout << "using the naive engine. Warning: works only for small numbers of actions " << endl;
      res = c.naiveSynth(net.arctlForm);
    }
    gettimeofday(&timr, NULL);
    double synend = timr.tv_sec + (timr.tv_usec/1000000.0);
    cout << "Done (" << synend - synstart << " sec., " << manager.ReadMemoryInUse()/1048576.0 << "MB BDD memory)" << endl;
    
    BDD st = net.initGlobal * net.allVarSel;

    cout << "Outcome: ";
    BDD initres = (res * st);
    if(initres == st) cout << "always true" << endl;
    else if(initres == manager.bddZero()) cout << "always false" << endl;
    else {
      initres = initres.ExistAbstract(net.globalStatesCube);
      cout << "result is parametric with " << (initres).CountMinterm(net.nvars) << " valuations"  << endl; 
        c.net.display_some_witness(initres, all, minimal);
        }

    return 0;
    }
