/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef CHECKER_H
#define CHECKER_H

#include <set>
#include <string>
#include "model.h"
#include "ltsNetCppWrapper.h"
#include "modelTools.h"
#include "cuddObj.hh"
#include "checker.h"

typedef long long int naivegroupsize;

/* The pmARCTL model checking engine */
class Checker {

    public:

        Network& net;
        Cudd& manager;

        Checker(Network& networkv, Cudd& managerv);

        ~Checker() { };

        /* Symbolic verification procedures. All these take and return nonprimed vars.
           The ones listed here are non-parametric, i.e., they include a fixed sets
           of actions. The pstates BDD can contain parametric states, i.e., the state
           encodings augmented with action variables. The pstates can therefore be
           a result of parametic call and arbitrary nestings are possible. (The same
           goes for qstates, of course).*/

        BDD check_EX(set<string> actions, BDD pstates);
        BDD check_EF(set<string> actions, BDD pstates);
        BDD check_EU(set<string> actions, BDD pstates, BDD qstates);
        BDD check_EGom(set<string> actions, BDD pstates);  //this is the one with infinite paths only
        BDD check_EG(set<string> actions, BDD pstates);
        BDD check_AX(set<string> actions, BDD pstates);
        BDD check_AF(set<string> actions, BDD pstates);
        BDD check_AG(set<string> actions, BDD pstates);
        BDD check_AGom(set<string> actions, BDD pstates);
        BDD check_AU(set<string> actions, BDD pstates, BDD qstates);

        /* Symbolic verification procedures for parametric modalities */
        BDD check_EX(string parVariable, BDD pstates);
        BDD check_EF(string parVariable, BDD pstates);
        BDD check_EU(string parVariable, BDD pstates, BDD qstates);
        BDD check_EGom(string parVariable, BDD pstates);
        BDD check_EG(string parVariable, BDD pstates);
        BDD check_AX(string parVariable, BDD pstates);
        BDD check_AF(string parVariable, BDD pstates);
        BDD check_AG(string parVariable, BDD pstates);
        BDD check_AGom(string parVariable, BDD pstates);
        BDD check_AU(string parVariable, BDD pstates, BDD qstates);

        /* Returns descriptor of all parameter valuations (coded using
           action variables) under which formula holds in the initial
           global state of the net network */
        BDD synthesise(formTree* formula);

	/* For benchmarking purposes: iterates over all possible variable 
	   substitutions in formula, performing  nonparametric checks. */
	BDD naiveSynth(formTree* formula);

        /* Return immediate deadlocks (states with nonprimed encoding) for
           the nonparametric input relation */
        BDD deadOnes(BDD rel);

        /* Return immediate deadlocks for the parametric input relation
           encoded using parVariable. The returned BDD is also
           parVariable-encoded. */
        BDD parametricDeadOnes(BDD* paraRel = NULL, string* parVariable = NULL);

 private:
	
	/* Called recursively by naiveSynth, rec. goes through all substitutions */
	void recNextCall(naivegroupsize sumVal, int depth, int bits, BDD& resbdd);

    };

#endif
