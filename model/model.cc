/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include <assert.h>
#include "model.h"
#include "ltsNetCppWrapper.h"
#include "modelTools.h"
#include "cuddObj.hh"

/* A single manager is used in this program. Build it in main function file. */
extern Cudd manager;
extern formTree* formula; /* from ast.c */
extern vectr* limits;

Model::Model(): bits(-1) { }


void Model::encodeModel(bool verbose = false) {
    if(verbose) cout << "Encoding component " << name;
    int sspaceSize = states.size();
    bits = ceil(log(sspaceSize) / log(2));
    if(verbose) cout << " of " << sspaceSize << " states. Using " << bits << " bits."  << endl;

    //prepare primed and nonprimed BDD variables
    stateBDDVarsCube = manager.bddOne();
    stateBDDVarsPrimedCube = manager.bddOne();
    for(int i = 0; i < bits; ++i) {
        BDD newNonPrimedVar = manager.bddVar();
        stateBDDVars.push_back(newNonPrimedVar);
        stateBDDVarsCube *= newNonPrimedVar;

        BDD newPrimedVar = manager.bddVar();
        stateBDDVarsPrimed.push_back(newPrimedVar);
        stateBDDVarsPrimedCube *= newPrimedVar;
        }

    //encode states
    int ctr = 0;
    for(set<string>::const_iterator iter = states.begin(); iter != states.end();
            ++iter, ++ctr) {
        //insert (state, encoding) to statesBDDsMap
        string stateName = *iter;
        BDD stateEncoding = intToBDD(ctr, stateBDDVars);
        state2encoding[*iter] = stateEncoding;
        }
    assert(state2encoding.size() == states.size());

    //encode propositions
    if(verbose) cout << "Encoding propositions:" << endl;
    for(propositionToStatesSet::const_iterator iter = propToStates.begin();
            iter != propToStates.end(); ++iter) {
        string propositionName = iter->first;
        BDD setEncoding = manager.bddZero();
        if(verbose) cout << propositionName << " marks: "<< endl;
        for(set<string>::const_iterator stiter = (iter->second).begin();
                stiter != (iter->second).end(); ++stiter) { //go through states marked with propositionName
            setEncoding += state2encoding[*stiter]; //encode the set
            if(verbose) cout << *stiter << endl;
            }
        proposition2encodedSetOfStates[propositionName] = setEncoding;
        }

    //encode the initial state
    initStateBDD = proposition2encodedSetOfStates[INITIAL];

    //encode the labels and transitions
    if(verbose) cout << "Encoding action labels and transitions..." << endl;
    for(labelToTransSet::const_iterator iter = labelToTrans.begin();
            iter != labelToTrans.end(); ++iter) {

        string labelName = (*iter).first;
        actions.insert(labelName);
        if(verbose) cout << labelName << " labels:" << endl;

        //gather transitions for the current label
        actionToStates[labelName] = manager.bddZero();
        for(transSet::const_iterator triter = (*iter).second.begin();
                triter != (*iter).second.end(); ++triter) {
            string leftSideName = (*triter).first;
            string rightSideName = (*triter).second;

            if(verbose) cout << "(" << leftSideName << "," << rightSideName << ")" << endl;

            BDD leftSideBDD = getStateBDD(leftSideName);
            BDD rightSideBDD = getStateBDDPrimed(rightSideName);
            //join (source, target)
            BDD sourceTarget = leftSideBDD * rightSideBDD;
            //build transition relation
            actionToStates[labelName] += leftSideBDD * rightSideBDD;
            }
        }

    }


inline BDD Model::getStateBDD(string stateName) {
    return state2encoding[stateName];
    }


inline BDD Model::getTransitionsLabeledWith(string label) {
    if(actionToStates.count(label) == 0) return manager.bddZero();
    else return actionToStates[label];
    }


inline BDD Model::getStatesMarkedWith(string proposition) {
    if(proposition2encodedSetOfStates.count(proposition) == 0) return manager.bddZero();
    else return proposition2encodedSetOfStates[proposition];
    }


inline BDD Model::getStateBDDPrimed(string stateName) {
    return state2encoding[stateName].SwapVariables(stateBDDVars, stateBDDVarsPrimed);
    }


ostream& operator<<(ostream& out, const Model& model) {
    out << "Model: " << model.name << endl;

    out << "States: \n";
    for(set<string>::const_iterator iter = model.states.begin();
            iter != model.states.end(); ++iter) {
        cout << "\t" << *iter << endl;
        }

    out << "Initial state: " << model.initStateName << endl;

    out << "Label ";
    for(propositionToStatesSet::const_iterator iter = model.propToStates.begin();
            iter != model.propToStates.end(); ++iter) {
        cout << (*iter).first << " marks:" << endl;

        for(set<string>::const_iterator stiter = (*iter).second.begin();
                stiter != (*iter).second.end(); ++stiter) {
            cout << "\t" << *stiter << endl;
            }

        }

    out << "Transitions: \n";
    for(labelToTransSet::const_iterator iter = model.labelToTrans.begin();
            iter != model.labelToTrans.end(); ++iter) {
        cout << (*iter).first << " marks:" << endl;

        for(transSet::const_iterator triter = (*iter).second.begin();
                triter != (*iter).second.end(); ++triter) {
            cout << "\t" << (*triter).first << " --> " << (*triter).second << endl;
            }

        }

    return out;
    }


Network::Network(): nvars(0), allVarSel(manager.bddOne()) {  }


Network::~Network() {
    for(unsigned int cno = 0; cno < components.size(); ++cno) { //free components
        delete components[cno];
        }

    /* clear formula */
    freeFormTree(arctlForm);
    }


bool Network::buildNetwork(char* filename, bool verbose = false) {
    if(verbose) cout << "> Reading the network from: " << filename << endl;

    Parser p;
    if(!p.initParser(filename)) return false;

    arctlForm = formula;

    globalStatesCube = manager.bddOne();
    globalStatesPrimedCube = manager.bddOne();
    initGlobal = manager.bddOne();
    bits = 0;
    for(int cno = 0; cno < p.components; ++cno) { //build all components
        components.push_back(p.buildModel(cno));
        components[cno]->encodeModel(verbose);
        if(verbose) cout << "Done." << endl;
        //gather BDD variables from all components
        for(unsigned int i = 0; i < components[cno]->stateBDDVars.size(); ++i) {
            BDD var = components[cno]->stateBDDVars[i];
            allStateBDDVars.push_back(var);
            }
        for(unsigned int i = 0; i < components[cno]->stateBDDVarsPrimed.size(); ++i) {
            BDD pvar = components[cno]->stateBDDVarsPrimed[i];
            allStateBDDVarsPrimed.push_back(pvar);
            }
        //build cubes
        globalStatesCube *= components[cno]->stateBDDVarsCube;
        globalStatesPrimedCube *= components[cno]->stateBDDVarsPrimedCube;
        //initial global state
        initGlobal *= components[cno]->initStateBDD;
        //collect number of bits for global state encoding
        bits += components[cno]->bits;
        }

    for(vector<Model*>::const_iterator iter = components.begin();
            iter != components.end(); ++iter) {
        //gather all action labels from components
        actions.insert(((*iter)->actions).begin(), ((*iter)->actions).end());

        for(stringToBDD::const_iterator it = (*iter)->proposition2encodedSetOfStates.begin();
                it != (*iter)->proposition2encodedSetOfStates.end(); ++it) {
            propositions.insert(it->first);
            }

        }


    //Concerns limits-enabled models only. Gather actions allowed in synthesis. */
    vectr* alacts = limits;

    set<string> constactionshlp(actions.begin(), actions.end());
    if(limits != NULL) {
        for(int i = 0; i < limits->elements; ++i) {
            string alact = string((char*) get(alacts, i));
            if(actions.count(alact) == 0) {
                cout << "Unknown action in limits: " << alact << endl;
                return false;
                }
            allowedToRemoveActions.push_back(alact);
            constactionshlp.erase(alact);
            }

        freeVectr(alacts, free);
        constactions.resize(constactionshlp.size());
        copy(constactionshlp.begin(), constactionshlp.end(), constactions.begin());
        }


    //create the BDD for the global relation
    transition = manager.bddZero();
    actionsCube = manager.bddOne();
    negatedActionsCube = manager.bddOne();
    if(verbose) cout << "> Building the global relation.";
    for(set<string>::const_iterator iter = actions.begin();
            iter != actions.end(); ++iter) { //for each action...

        string action = *iter;
        BDD actionBDD = manager.bddVar();
        BDD transitionBDD = actionBDD;
        actionsCube *= actionBDD;
        negatedActionsCube *= !actionBDD;

        action2BDD[action] = actionBDD;
	
	if(limits != NULL && isActionConst(action)) {
	  transitionBDD = manager.bddOne(); //constant actions need no vars
	}
        if(verbose) cout << "\nAction " << action << " found in: ";

        for(int cno = 0; cno < p.components; ++cno) { //for each component...
            Model* component = components[cno];
            BDD componentTransitionBDD = component->getTransitionsLabeledWith(action);
            if(componentTransitionBDD != manager.bddZero()) { //component recognises action
                transitionBDD *= componentTransitionBDD;
                if(verbose) cout << component->name << " ";
                }
            else { //component doesn't recognise the action
                //add self-loops for unknown action
                BDD selfLoop = manager.bddZero();
                for(set<string>::const_iterator ls = component->states.begin();
                        ls != component->states.end(); ++ls) {
                    selfLoop += component->getStateBDD(*ls) * component->getStateBDDPrimed(*ls);
                    }
                transitionBDD *= selfLoop;
                }
            }

        transition += transitionBDD; //update global transition with current action's transitions
        }
    if(verbose) cout << endl;

    //check if formula contains known actions and propositions;
    if(!validateForm(formula)) return false;

    //reachable states set
    computeReachableStates(verbose);

    return true;
    }


void Network::computeReachableStates(bool verbose) {
    if(verbose) cout << "> Computing reachable statespace... " << endl;

    statespace = initGlobal;
    BDD previous = manager.bddZero();
    int depth = 0;
    while(previous != statespace) {
        previous = statespace;
        BDD nextStep = (transition * statespace).SwapVariables(allStateBDDVars, allStateBDDVarsPrimed);
        nextStep = nextStep.ExistAbstract(globalStatesPrimedCube * actionsCube);
        statespace += nextStep;
        ++depth;
        }

    if(verbose) cout << "Fixpoint at depth: " << depth << endl;
    }


BDD Network::globalStateName2BDD(vector<string>& statev) {
    BDD global = manager.bddOne();
    for(unsigned int i = 0; i < components.size(); ++i) {
        global *= components[i]->getStateBDD(statev[i]);
        }
    return global;
    }


vector<string> Network::getEnabledActions(vector<string>& statev) {
    vector<string> enabled;
    BDD state = globalStateName2BDD(statev);

    BDD outActions = (state * transition).ExistAbstract(globalStatesCube * globalStatesPrimedCube);

    if(outActions != manager.bddZero()) {

        for(set<string>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
            if(outActions.ExistAbstract(getActionBDD(*it)) != outActions) {
                enabled.push_back(*it);
                }
            }

        }

    return enabled;
    }


BDD Network::fire(string action, vector<string> globalState) {
    BDD actvar = getActionBDD(action);
    BDD globalStateBDD = globalStateName2BDD(globalState);

    BDD negcube = negatedActionsCube.ExistAbstract(actvar); //!action_1 * .. * !action_n with action omitted

    BDD result = (globalStateBDD * transition * negcube).ExistAbstract(actionsCube); //pairs (globalstate,r') s.t. (globalstate,action,r')
    result = result.ExistAbstract(globalStatesCube);
    result = result.SwapVariables(allStateBDDVars, allStateBDDVarsPrimed);

    return result;
    }


inline BDD Network::getActionBDD(string actionName) {
    return action2BDD[actionName];
    }


BDD Network::getActionBDD(string actionName, string parVar) {
    return changeActionVars(getActionBDD(actionName), parVar, origVarName);
    }


string Network::getActionName(BDD action) {
    for(stringToBDD::const_iterator iter = action2BDD.begin();
            iter != action2BDD.end(); ++iter) {
        string actionName = iter->first;
        BDD actionBDD = iter->second;
        if(action == actionBDD) return actionName;
        }
    cerr << "Error: unknown action variable!" << endl;
    exit(1);
    }


string Network::getActionName(BDD action, string parVar) {
    return getActionName(changeActionVars(action, parVar, origVarName));
    }


inline BDD Network::getTransition() {
    return transition;
    }


bool Network::buildNetwork(string filename, bool verbose = false) {
    return buildNetwork((char*) filename.c_str(), verbose);
    }


void Network::simulate() {

    vector< vector<string> > gst = globalStateSetDescription(initGlobal);
    int statesel = -1;
    int actionsel = -1;
    int statesNo;

    do {
        statesNo = (int) gst.size();

        for(int i = 0; i < statesNo; ++i) { //print states
            cout << "*-[" << i << "] ( ";
            for(unsigned int j = 0; j < gst[i].size(); ++j) cout << gst[i][j] << " ";
            vector<string> enact = getEnabledActions(gst[i]);
            if(enact.size() > 0) cout << ") with enabled actions: ";
            else cout << ") with no enabled actions";

            for(int k = 0; k < (int) enact.size(); ++k) cout << enact[k] << " ";
            cout << endl;
            }

        cout << "Select state (-1 to quit): ";
        scanf(" %i", &statesel);

        if(statesel == -1) return;
        if(statesel >= 0 && statesel < statesNo) { //print actions

            vector<string> enact = getEnabledActions(gst[statesel]);
            int enactNo = (int) enact.size();

            for(int k = 0; k < enactNo; ++k) cout  << "--[" << k << "] " << enact[k] << endl;
            if(enact.size() == 0) {
                cout << "No actions to fire. Quitting on deadlock." << endl;
                return;
                }
            cout << "Select action to fire: ";
            scanf(" %i", &actionsel);
            if(actionsel >= 0 && actionsel < enactNo) { //fire action, update states
                cout << "Fired " << enact[actionsel] << " from ( ";
                for(unsigned int j = 0; j < gst[statesel].size(); ++j) cout << gst[statesel][j] << " ";
                cout << ")" << endl;

                BDD successors = fire(enact[actionsel], gst[statesel]);

                gst = globalStateSetDescription(successors);

                }

            }

        }
    while(true);

    }


vector< vector<string> > Network::globalStateSetDescription(BDD globalsSet) {

    //limit used actions to seen in the set only
    vector< vector<string> > knownLocalStates(components.size()); //i--th component -> local states in global
    vector< vector<string> > goodLocalStates; //good states are in 'rows'

    for(unsigned int i = 0; i < components.size(); ++i) { //for each component...
        for(stringToBDD::const_iterator stiter = components[i]->state2encoding.begin();
                stiter != components[i]->state2encoding.end(); ++stiter) { //gather its relevant local states...
            string state = stiter->first;
            BDD encoding = stiter->second;

            if(encoding * globalsSet != manager.bddZero()) knownLocalStates[i].push_back(state);
            }
        }

    //recursive check for relevant global states
    vector<string> helper;
    recursiveStatesDescription(globalsSet, knownLocalStates, 0, helper, goodLocalStates);

    return goodLocalStates;
    }


void Network::globalStateSetDisplay(BDD globalsSet) {
    vector< vector<string> > res = globalStateSetDescription(globalsSet);
    for(vector< vector<string> >::const_iterator iter = res.begin(); iter != res.end(); ++iter) {
        cout << "( ";
        for(vector<string>::const_iterator it = iter->begin(); it != iter->end(); ++it) cout << *it << " ";
        cout << ")" << endl;
        }
    }


void Network::recursiveStatesDescription(BDD& globalsSet, vector< vector<string> >& knownLocals,
        unsigned int position, vector<string> currstring,
        vector< vector<string> >& goodLocalStates) {
    if(position >= knownLocals.size()) {
        //build and check current state
        BDD currentState = manager.bddOne();
        for(unsigned int i = 0; i < currstring.size(); ++i) {
            currentState *= components[i]->getStateBDD(currstring[i]);
            }
        if(currentState * globalsSet == currentState ) goodLocalStates.push_back(currstring);
        }
    else { //recursive calls
        for(unsigned int i = 0; i < knownLocals[position].size(); ++i) {
            vector<string> loc = currstring;
            loc.push_back(knownLocals[position][i]);
            recursiveStatesDescription(globalsSet, knownLocals, position + 1, loc, goodLocalStates);
            }
        }
    }


bool Network::validateForm(formTree* ft) {
    static bool guard = false;

    if(ft == NULL) return true;

    if(ft->path_selector == EXISTS || ft->path_selector == ALWAYS) { //nonparametric modal
        //check if all listed actions are ok
        string actname;
        for(int i = 0; i < (ft->elements)->elements; ++i) {
            actname = string((char*)get(ft->elements, i));
            if(actions.count(actname) == 0) {
                cerr << "Formula error, no such action: " << actname << endl;
                return false;
                }
            }
        }
    else if(ft->path_selector == EXISTSPARA || ft->path_selector == ALWAYSPARA) { //parametric modal
        //build/gather BDD variables for actions
        string parvarname = ft->varName;
        vector<BDD> vv;

	//act_i + ... + act_k encoded for curr. var. which are not constant
        BDD locvar = manager.bddZero();
	//act_i * ... * act_k encoded for curr. var. which are constant
	BDD loccvar = manager.bddOne();

        if(parVariableNameToBDDVec.count(parvarname) == 0) {
            parVars.insert(parvarname);
            for(set<string>::const_iterator act = actions.begin();
                    act != actions.end(); ++act) { //collect BDD variables for parvarname
                BDD currvar;
                if(!guard) {
                    currvar = getActionBDD(*act);
                    origVarName = parvarname;
                    }
                else currvar = manager.bddVar();
		if((limits == NULL) || ((limits != NULL) && !isActionConst(*act)))
		  locvar += currvar;
		else 
		  loccvar *= currvar;
                vv.push_back(currvar);
                }
            parVariableNameToBDDVec[parvarname] = vv;
            allVarSel *= locvar * loccvar;
            guard = true;
            nvars += actions.size();
	    }
        }
    else if(ft->path_selector == NONEPARA) { //booleans
        if(ft->opertr == MNONE) { //check if propositions are ok
            string proposition = ft->varName;
            if(propositions.count(proposition) == 0) {
                cerr << "Formula error, no such proposition: " << proposition << endl;
                return false;
                }
            else return true;
            }
        }
    else {
        cerr << "Formula validation error: check the source for bugs!" << endl;
        return false;
        }

    return validateForm(ft->left) && validateForm(ft->right);
    }


BDD Network::getStatesMarkedWith(string proposition, bool disjunctive) {
    if(components.size() == 0) return manager.bddZero();

    BDD result;
    if(disjunctive) result = manager.bddZero();
    else result = manager.bddOne();

    for(unsigned int i = 0; i < components.size(); ++i) { //for each component...
        if(disjunctive) result += components[i]->getStatesMarkedWith(proposition);
        else result *= components[i]->getStatesMarkedWith(proposition);
        }

    return result;
    }


BDD Network::getLabeledTransitions(set<string> actionsSet) {
    BDD result = manager.bddZero();

    for(set<string>::const_iterator it = actionsSet.begin(); it != actionsSet.end(); ++it) { //for an action
        BDD actionTrans = manager.bddOne();
        string action = *it;

        for(unsigned int cno = 0; cno < components.size(); ++cno) { //for each component
            Model* component = components[cno];
            BDD compTrans = component->getTransitionsLabeledWith(action);
            if(compTrans != manager.bddZero()) actionTrans *= compTrans; //gather transitions for action if known
            else {//stay in place for unknown actions
                BDD selfLoop = manager.bddZero();
                for(set<string>::const_iterator ls = component->states.begin();
                        ls != component->states.end(); ++ls) {
                    selfLoop += component->getStateBDD(*ls) * component->getStateBDDPrimed(*ls);
                    }
                actionTrans *= selfLoop;
                }
            }

        result += actionTrans;
        }

    return result;
    }


BDD Network::changeActionVars(BDD bddv, string parVar1, string parVar2) {
    return bddv.SwapVariables(parVariableNameToBDDVec[parVar1], parVariableNameToBDDVec[parVar2]);
    }


BDD Network::changeActionVars(BDD bddv, string parVar1) {
    return changeActionVars(bddv, parVar1, origVarName);
    }


bool Network::isActionConst(string actname) {
    return find(constactions.begin(), constactions.end(), actname) != constactions.end(); 
}


void Network::display_some_witness(BDD result, bool all = false, bool minimal = false) {
    cout << "Printing out example valuations" << endl;
    int ctr = 0;

    BDD resultcp = result;

    //collect all action vars
    vector<BDD> allParVars;
    for(stringToBDDvect::const_iterator iter = parVariableNameToBDDVec.begin();
            iter != parVariableNameToBDDVec.end(); ++iter) {
        vector<BDD> pvarv = iter->second;
        allParVars.insert(allParVars.end(), pvarv.begin(), pvarv.end());
        }
    //print only prime implicants - i.e., minimal valuations
    if(minimal) {
      //check if the formula is monotone
      cout << "test " << endl;
      //display prime implicants
    }
    return;
    
    //iterate over all parameter valuations in result
    while(resultcp != manager.bddZero()) {
        cout << "(" << ++ctr << ")" << endl;
        BDD goodVal = resultcp.PickOneMinterm(allParVars);

        //display valuation
	displayValuation(goodVal);

        resultcp *= !goodVal; //block goodVal

        if(!all) {
            cout << "Display another? [y/n/a]: ";
            char qu;
            scanf(" %c", &qu);
            if(qu == 'n' || qu == 'N') exit(0);
            if(qu == 'a') all = true;
            }
        }

    }

BDD Network::valuation2BDD(map<string, vector<string> > valuation) {
    //Note - it is assumed that valuations don't contain empty sets!
    BDD result = manager.bddOne();
    for(set<string>::const_iterator it = parVars.begin(); it != parVars.end(); ++it) { //for a par. variable
        string varname = *it;
        if(valuation[varname].size() == 0) {
            cerr << "Panic: valuation can't be empty! (it was for " << varname << ")" << endl;
            exit(1);
            }
        for(set<string>::const_iterator ii = actions.begin(); ii != actions.end(); ++ii) {//encode a set
            string action = *ii;
            BDD actionBDD = getActionBDD(action, varname);

            if(find(valuation[varname].begin(), valuation[varname].end(), action) != valuation[varname].end()) {
                result *= actionBDD;
                }
            else result *= !actionBDD;
            }
        }

    return result;
    }


void Network::displayValuation(BDD& goodVal) {
  for(stringToBDDvect::const_iterator iter = parVariableNameToBDDVec.begin();
      iter != parVariableNameToBDDVec.end(); ++iter) {
    string varName = iter->first;
    vector<BDD> pvarv = iter->second;
    cout << varName << " -> { ";
    bool nonempty = false;
    for(unsigned int i = 0; i < pvarv.size(); ++i) {
      if(pvarv[i] * goodVal != manager.bddZero()) {
	cout << getActionName(pvarv[i], varName)  << ", ";;
	nonempty = true;
      }
    }
    if(nonempty) cout << "\b\b";
    cout <<  " };" << endl;
  }
}
