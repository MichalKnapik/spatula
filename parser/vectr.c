/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "vectr.h"
#include "graph.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


vectr* newVectr() {
    vectr* v = malloc(sizeof(vectr));
    v->elements = 0;
    v->capacity = VECSTEP;
    v->data = malloc(sizeof(void*) * v->capacity);
    return v;
    }


void append(vectr* v, void* elmnt) {
    if(v->capacity == v->elements) {
        int newCap = v->capacity + VECSTEP;
        void** bfr = malloc(sizeof(void*) * newCap);
        memcpy(bfr, v->data, v->elements * (sizeof(void*)));
        free(v->data);
        v->data = bfr;
        v->capacity = newCap;
        }
    v->data[v->elements++] = elmnt;
    }


void* get(vectr* v, int i) {
    return v->data[i];
    }


void freeVectr(vectr* v, void (*freefn)(void*)) {
    int i = 0;
    if(freefn != NULL) for(i = 0; i < v->elements; ++i) {
            freefn(v->data[i]);
            }
    free(v->data);
    free(v);
    }


void clearNodeVectr(void* v) {
    vectr* w = (vectr*) v;
    int i = 0;
    for(i = 0; i < w->elements; ++i) {
        free(w->data[i]);
        }
    free(w->data);
    free(w);
    }

void clearUnledgeVectr(void* v) {
    vectr* w = (vectr*) v;
    int i = 0;
    for(i = 0; i < w->elements; ++i) {
        freeUnledge(w->data[i]);
        }
    free(w->data);
    free(w);
    }


mmap* newMmap() {
    mmap* m = malloc(sizeof(mmap));
    m->keys = newVectr();
    m->vals = newVectr();
    return m;
    }


int findIndex(mmap* m, char* key) {
    int i = 0;
    for(i = 0; i < (m->keys)->elements; ++i) {
        if( strcmp((char*)get(m->keys,i), key) == 0 ) return i;
        }
    return -1;
    }


void mapInsert(mmap* m, char* key, void* value, int (*cmpfn)(void* v1, void* v2), void (*freefn)(void*)) {
    /* NULL values allow for set emulation */
    int fnd = findIndex(m, key);

    if(fnd == -1) { /* key not found */
        append(m->keys, key);
        if(value != NULL) {
            vectr* v = newVectr();
            append(v, value);
            append(m->vals, v);
            }
        }
    else {   /* key found */
        free(key); /* dodgy design - assumes that key is strdup-ed */
        if(value != NULL) {
            vectr* valvctr = get(m->vals, fnd);
            if(cmpfn != NULL) {
                int vctr = valvctr->elements;
                while(--vctr >= 0) {
                    if(cmpfn(value, get(valvctr, vctr)) == 0) {
                        freefn(value);
                        return;
                        }
                    }
                }
            append(valvctr, value);
            }
        }
    }


void freeMap(mmap* m, void (*freefn)(void*)) {
    /* both keys and values lie on the heap */
    freeVectr(m->keys, free);
    freeVectr(m->vals, freefn);
    free(m);
    }
