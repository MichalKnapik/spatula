/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef LTSNETCPPWRAPPER_H
#define LTSNETCPPWRAPPER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "model.h"

static const string INITIAL = "initial";

class Parser {

    public:

        int components;

        /* Dummy, run initParser */
        Parser();

        /* Also performs cleanup */
        ~Parser();

        /* Try to load and parse a model file. Perform initial cleanup if needed,
        that is: the parser can be reset to another file and reused. */
        bool initParser(char* fname);

        /* Build and return the model for i--th component */
        Model* buildModel(int compNo);

    private:
        FILE* yyinfile;

    };

#endif








