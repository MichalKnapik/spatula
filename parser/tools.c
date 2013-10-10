/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void terror(int line, const char* form, const char* msg, int val) {
    if(line >= 0) fprintf(stderr, "Line %d: ", line);
    fprintf(stderr, form, msg, val);
    exit(1);
    }
