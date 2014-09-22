/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/


#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <vector>
#include "ast.h"
#include "cuddObj.hh"

using std::string;
using std::set;
using std::map;
using std::pair;
using std::ostream;
using std::ostringstream;
using std::vector;
using std::cout;
using std::cin;
using std::find;
using std::getline;
using std::cerr;
using std::endl;
using std::copy;

typedef map<string, set<string> > propositionToStatesSet;
typedef set<pair<string, string> > transSet;
typedef map<string, set<pair<string, string> > > labelToTransSet;
typedef map<string, BDD> labelToTransSetBDD;
typedef map<string, BDD> stringToBDD;
typedef map<string, vector<BDD> > stringToBDDvect;

/* A single component of the network */

class Model {

    public:

        /* The constructor doesn't do anything. Use the model returned by Parser. */
        Model();

        /* Builds all BDDs for the model. Call this before playing with the model. */
        void encodeModel(bool verbose);

        /* Model's name */
        string name;

        /* Initial state name */
        string initStateName;

        /* Initial state BDD */
        BDD initStateBDD;

        /* Returns nonprimed BDD state encoding */
        BDD getStateBDD(string stateName);

        /* Returns nonprimed BDD state encoding */
        BDD getStateBDDPrimed(string stateName);

        /* Returns the BDD of nonprimed-vars-encoded states marked with
           proposition. Returns 0 for emptyset. */
        BDD getStatesMarkedWith(string proposition);

        /* Return the BDD of transitions with a given action label.
           Returns 0 for emptyset. */
        BDD getTransitionsLabeledWith(string label);

        /* The known action labels */
        set<string> actions;

        /* The set of all states names */
        set<string> states;

    private:

        /* The number of bits used to encode states. Computed while encoding. */
        int bits;

        /* Map from propositions to their marked states' encoding (nonprimed) */
        stringToBDD proposition2encodedSetOfStates;

        /* Map from state names to their encoded BDDs (nonprimed) */
        stringToBDD state2encoding;

        /* State BDD variables, nonprimed */
        vector<BDD> stateBDDVars;

        /* State BDD variables, primed */
        vector<BDD> stateBDDVarsPrimed;

        /* Map from propositions to sets of labeled states */
        propositionToStatesSet propToStates;

        /* Map from labels to sets of transitions */
        labelToTransSet labelToTrans;

        /* Map from labels to encoded sets of transitions */
        labelToTransSetBDD actionToStates;

        /* Cubes for abstracts */
        BDD stateBDDVarsCube;
        BDD stateBDDVarsPrimedCube;
        /* ------------------- */

        friend ostream& operator<<(ostream& out, const Model& model);

        friend class Parser;

        friend class Network;

        friend class Checker;

    };


/* The network of models */

class Network {

    public:

        Network();
        ~Network();

        /* The formula to be checked (see ast.h) */
        formTree* arctlForm;

	/* Actions allowed in valuations */
	vector<string> allowedToRemoveActions;

        /* The number of bits used to encode global states. */
        int bits;

        /* The initial global state */
        BDD initGlobal;

        /* Nonprimed-vars encoded set of reachable global states */
        BDD statespace;

	/* Number of actions times number of action variables */
	int nvars;

        /* Action labels known by any of the models */
        set<string> actions;

        /* Propositions marking some state in the models */
        set<string> propositions;

        /* Names of parametric variables */
        set<string> parVars;

        /* Builds all components of the model network */
        bool buildNetwork(string filename, bool verbose);
        bool buildNetwork(char* filename, bool verbose);

        /* Returns the set of encoded triples (global origin, action, global target) */
        BDD getTransition();

        /* Returns the action's BDD variable name for the virtual first
           parametric variable */
        string getActionName(BDD action);

        /* Returns the BDD variable for action, for the virtual first
           parametric variable */
        BDD getActionBDD(string actionName);

        /* Returns the action's BDD variable name for the parVar
           parametric variable */
        string getActionName(BDD action, string parVar);

        /* Returns the BDD variable for action, for the parVar
           parametric variable */
        BDD getActionBDD(string actionName, string parVar);

        /* Returns the BDD of nonprimed-vars-encoded states marked with
           proposition. Returns 0 for emptyset. The semantics switch changes
	   between disjunctive and conjunctive (default) markings, 
	   e.g., if p marks state X in one component and state Y but not Z
	   in other, then (X,Y) is labeled by p in component join under both
	   semantics, but (X, Z) is labeled by p in disjunctive semantics
	   only. */
        BDD getStatesMarkedWith(string proposition, bool disjunctive = false);

        /* Runs an interactive simulation of the network */
        void simulate();

        /* Returns the set of pairs (x, x') such that (x,a,x') is a global transition
           for some a in actionsSet */ 
        BDD getLabeledTransitions(set<string> actionsSet);

        /* -- Simulation tools -- */

        /* Returns the list of global states in given set (nonprime-encoded) for the
           purpose of simulation. Warning: this is VERY inefficient but saves space.
           Use for debugging and simulation only. */
        vector< vector<string> > globalStateSetDescription(BDD globalsSet);

        /* Displays globalStateSetDescription result */
        void globalStateSetDisplay(BDD globalsSet);

        /* Returns a vector of action names enabled for global state in vector
           (order of components property). For simulation purposes. */
        vector<string> getEnabledActions(vector<string>& statev);

        /* Fires action in globalState and returns the non-primed encoding of outcome */
        BDD fire(string action, vector<string> globalState);

        /* Builds the statespace */
        void computeReachableStates(bool verbose);

        /* Checks if the formula's actions and propositions are valid */
        bool validateForm(formTree* ft);

        /* Returns the BDD representation for global state in vector (order of components property) */
        BDD globalStateName2BDD(vector<string>& statev);

        /* Substitutes all parVar1 action variables in the input BDD with
           ones from parVar2 and vice versa */
        BDD changeActionVars(BDD bddv, string parVar1, string parVar2);

        /* Call of the above with parVar2 = origVarName */
        BDD changeActionVars(BDD bddv, string parVar1);

        /* Result is expected to be an encoding of some set of parameter valuations.
           This function prints them out. */
        void display_some_witness(BDD result, bool all, bool minimal);

	/* Encodes the input valuation as BDD. Used in naive verification */
	BDD valuation2BDD(map<string, vector<string> > valuation);

        /* -- Cubes for abstracts -- */
        BDD globalStatesCube;
        BDD globalStatesPrimedCube;
        BDD actionsCube;
        BDD negatedActionsCube; //!action_1 * ... * !action_n
        BDD allVarSel; //conjunction of disjunctions of actions for all vars
	
    private:

        /* The name of first encountered variable */
        string origVarName;

	/* Check if a given action can be switched on/off */
	bool isActionConst(string actname);

	/* Complement of allowedToRemoveActions w.r.t. to all actions */
	vector<string> constactions;

        /* Contains all built components of a network */
        vector<Model*> components;

        /* A recursive function called by globalStateSetDescription */
        void recursiveStatesDescription(BDD& globalsSet, vector< vector<string> >& knownLocals,
                                        unsigned int position, vector<string> currstring,
                                        vector< vector<string> >& goodLocalStates);

        /* Transition relation of the network. It consists of triples:
           (source, label, goal) where source and goal are log2 encoded
           using resp., nonprimed and primed vars, and label is a separate variable. */
        BDD transition;

        /* A map from an action name to its BDD */
        stringToBDD action2BDD;

        /* All state BDD variables, nonprimed */
        vector<BDD> allStateBDDVars;

        /* All state BDD variables, primed */
        vector<BDD> allStateBDDVarsPrimed;

        /* A map from parametric variable's name to its action BDD variables */
        stringToBDDvect parVariableNameToBDDVec;

	/* Display valuation in goodVal */
	void displayValuation(BDD& goodVal);

        friend class Checker;

    };

#endif
