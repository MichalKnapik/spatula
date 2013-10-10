/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef MODELTOOLS_H
#define MODELTOOLS_H

#include "cuddObj.hh"
#include "ast.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <set>

using std::vector;
using std::set;
using std::map;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

/* Returns BDD representation if value using variables from bitVars */
BDD intToBDD(int value, vector<BDD>& bitVars);

/* Converts c-string vectr* to a set of strings */
set<string> stvectr2stringSet(vectr* v);

/* Converts vector<string> to a vectr* */
vectr* strVector2vectr(vector<string> v);

/* Returns formula tree with variables substituted according to valuation */
formTree* substitute(formTree* source, map<string, vector<string> > valuation);

void displayValuation(map<string, vector<string> > valuation);

#endif
