/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef VECTR_H
#define VECTR_H

#ifdef __cplusplus
extern "C" {
#endif

#define VECSTEP 100

    /* Poor man's vector */
    typedef struct {
        int elements;
        int capacity;
        void** data;
        } vectr;

    /* Basic operations */
    vectr* newVectr();
    void append(vectr* v, void* elmnt);
    void* get(vectr* v, int i);

    /* The freefn function is called for each element of the vector */
    void freeVectr(vectr* v, void (*freefn)(void*));

    /* Used as freefn for vectors of char* */
    void clearNodeVectr(void* v);

    /* Used as freefn for vectors of unledges */
    void clearUnledgeVectr(void* v);

    /* Poor man's map from strings to set of values. A set of values
       (if using mapInsert to add elements) is assigned to a key.    */
    typedef struct {
        vectr* keys; /* strings only */
        vectr* vals; /* vectr of vectrs */
        } mmap;

    mmap* newMmap();

    int findIndex(mmap* m, char* key);

    /* Inserts value to a set associated with a given key. The cmpfn is a value comparator function
       (e.g. strcmp), the freefn is used to free values if needed. NULL value allows for set emulation.  */
    void mapInsert(mmap* m, char* key, void* value, int (*cmpfn)(void* v1, void* v2), void (*freefn)(void*));

    /* The freefn function is called for the vector of values */
    void freeMap(mmap* m, void (*freefn)(void*));

#ifdef __cplusplus
    }
#endif

#endif
