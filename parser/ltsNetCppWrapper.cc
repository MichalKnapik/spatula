/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "ltsNetCppWrapper.h"
#include "ast.h"
#include "graph.h"
#include "vectr.h"
#include "tools.h"
#include "model.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

/* Globals from ast.c and parser */
extern astTree* topTrees[COMPONENTSMAXNO];
extern char* compnames[COMPONENTSMAXNO];
extern int compCtr;
extern mmap* states;
extern mmap* propositions;
extern mmap* edges;
extern formTree* formula;

extern FILE* yyin;
extern "C" {
    int yyparse();
    }

using namespace std;


Parser::Parser() : yyinfile(NULL) {}


Parser::~Parser() {
    if(yyinfile != NULL) {
        cleanup();
        fclose(yyinfile);
        }
    }


bool Parser::initParser(char* fname) {
    if(yyinfile != NULL) {
        components = -1;
        cleanup();
        fclose(yyinfile);
        }

    yyinfile = fopen(fname, "r");
    yyin = yyinfile;

    if(yyinfile == NULL) {
        perror(fname);
        return false;
        }

    bool outcome = (yyparse() == 0)? true : false;
    if(outcome) {
        init();
        components = compCtr;
        }

    return outcome;
    }


Model* Parser::buildModel(int compNo) {
    if(compNo < 0 || compNo >= components || components < 0) return NULL;

    Model* model = new Model();

    init();
    evalModel(compNo);
    model->name = compnames[compNo];
    model->initStateName = "";

    /* inefficient, but convenient: copying stuff to STL collections */
    /* copy states to STL set */
    vectr* st = states->keys;
    int ctr = st->elements;
    while(--ctr >= 0) model->states.insert(string((char*) get(st, ctr)));

    /* copy propositions to STL map */
    vectr* prs = propositions->keys;
    int prctr = prs->elements;
    while(--prctr >= 0) {
        string currProp = (char*) get(prs, prctr);
        if( model->propToStates.count(currProp) == 0 ) {
            model->propToStates[currProp] = set<string>();
            }

        vectr* vls = (vectr*) get(propositions->vals, prctr);
        int ctr = vls->elements;
        while(--ctr >= 0) {
            string state4prop = (char*) get(vls, ctr);
            model->propToStates[currProp].insert(state4prop);
            //initial state - there can be only one
            if(currProp == INITIAL) {
                if(model->initStateName != "" || ctr > 0) {
                    cerr << "Error: more than one initial state in model " << model->name << endl;
                    exit(1);
                    }
                model->initStateName = state4prop;
                }
            }
        }
    if(model->initStateName == "") {
        cerr << "Error: no initial state in model " << model->name << endl;
        exit(1);
        }

    /* copy transitions to STL map */
    vectr* eprs = edges->keys;
    int eprctr = eprs->elements;
    while(--eprctr >= 0) {
        string currLabel = (char*) get(eprs, eprctr); // take a label...
        if( model->labelToTrans.count(currLabel) == 0 ) {
            model->labelToTrans[currLabel] = set< pair<string, string> >();
            }

        vectr* evls = (vectr*) get(edges->vals, eprctr);
        int ectr = evls->elements;
        while(--ectr >= 0) { // ...and gather its transitions
            unledge* ul = (unledge*) get(evls, ectr);
            pair<string, string> trans(ul->st, ul->end);
            model->labelToTrans[currLabel].insert(trans);
            }

        }

    return model;
    }
