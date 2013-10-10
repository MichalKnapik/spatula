/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "graph.h"
#include "vectr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void freeUnledge(void* ul) {
    free(((unledge*) ul)->st);
    free(((unledge*) ul)->end);
    free(ul);
    }


unledge* newUnledge(char* st, char* end) {
    unledge* ul = malloc(sizeof(unledge));
    ul->st = st;
    ul->end = end;
    return ul;
    }


int unledgeEq(unledge* un1, unledge* un2) {
    int steq = strcmp(un1->st, un2->st);
    int endeq = strcmp(un1->end, un2->end);
    return steq*steq + endeq*endeq;
    }


void displayUnledge(unledge* unl) {
    printf("(%s %s)", unl->st, unl->end);
    }
