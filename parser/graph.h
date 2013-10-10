/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

    /* Note: nodes and propositions are identified by names */

    /* Unlabeled edge */
    typedef struct {
        char* st;
        char* end;
        } unledge;

    /* Returns 0 iff un1 is equal to un2 */
    int unledgeEq(unledge* un1, unledge* un2);

    void freeUnledge(void* ul);

    unledge* newUnledge(char* st, char* end);

    void displayUnledge(unledge* unl);

#ifdef __cplusplus
    }
#endif

#endif
