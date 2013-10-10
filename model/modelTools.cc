/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "modelTools.h"
#include <string.h>

extern Cudd manager;

BDD intToBDD(int value, vector<BDD>& bitVars) {
    BDD result = manager.bddOne();

    for(unsigned int i = 0; i < bitVars.size(); ++i) {
        char bit = value % 2;
        value = value / 2;
        if(bit == 1)
            result *= bitVars[i];
        else
            result *= !bitVars[i];
        }

    return result;
    }


set<string> stvectr2stringSet(vectr* v) {
    set<string> nset;
    for(int i = 0; i < v->elements; ++i) {
        string action = (char*) get(v, i);
        nset.insert(action);
        }

    return nset;
    }


vectr* strVector2vectr(vector<string> v) {
    vectr* vw = newVectr();
    for(vector<string>::const_iterator iv = v.begin(); iv != v.end(); ++iv) {
        char* c = strdup((*iv).c_str());
        append(vw, c);
        }

    return vw;
    }

formTree* substitute(formTree* source, map<string, vector<string> > valuation) {
    if(source == NULL) return NULL;

    int pathsel, oper;
    vectr* elmts = NULL;
    char* varbname = NULL;;
    if(source->path_selector == NONEPARA) { //boolean operations
        pathsel = source->path_selector;
        oper = source->opertr;
        if(source->opertr == MNONE) { //proposition
            varbname = strdup(source->varName);
            }
        }
    else if(source->path_selector == EXISTS || source->path_selector == ALWAYS) { //nonparametrics
        pathsel = source->path_selector;
        oper = source->opertr;
        elmts = newVectr();
        for(int i = 0; i < (source->elements)->elements; ++i) { //copy actions group
	    append(elmts, strdup((char*) get(source->elements, i)));
            }
        }
    else if(source->path_selector == EXISTSPARA || source->path_selector == ALWAYSPARA) { //parametrics
        string varname = source->varName;
        if(source->path_selector == EXISTSPARA) pathsel = EXISTS;
        else pathsel = ALWAYS;
        oper = source->opertr;
        elmts = strVector2vectr(valuation[varname]);
        }
    else {
        cerr << "Panic: internal error in substitute. Check source." << endl;
        exit(1);
        }


    formTree* left = substitute(source->left, valuation);
    formTree* right = substitute(source->right, valuation);
    formTree* newft = newFormTree(pathsel, oper, varbname, elmts);
    newft->left = left;
    newft->right = right;

    return newft;
    }


void displayValuation(map<string, vector<string> > valuation) {
  typedef map<string, vector<string> > msvs;
  for(msvs::const_iterator it = valuation.begin(); it != valuation.end(); ++it) {
    cout << it->first << " -> { ";
    for(vector<string>::const_iterator ii = (it->second).begin(); 
	ii != (it->second).end(); ++ii) {
      cout << *ii << ", ";
    }
    cout << "\b\b };" << endl;
  }
}
