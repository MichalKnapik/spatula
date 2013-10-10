/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#include "ast.h"
#include "graph.h"
#include "vectr.h"
#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ GLOBALS @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

astTree* topTrees[COMPONENTSMAXNO]; /* the array of the formula trees */

char* compnames[COMPONENTSMAXNO]; /* the names of the components */

formTree* formula; /* the formula to be verified */

vectr* limits = NULL; /* the actions allowed in synthesised valuations */

int compCtr = 0; /* counts the number of components */

int semantics = 0; /* The way the global states are marked; '1' stands
			       for disjunctive, i.e., a global state is marked by
			       'p' if any local component is. */

const char* opnames[] = {"NEWSCOPE", "DEFVAR", "TERMINAL", "EXPRESSION",
                         "RELATION", "IFOP", "IFBRNCH", "FOROP", "WHILEOP",
                         "DUMMY"
                        }; /* tree operation type names */

const char* typenames[] = {"INTEGER", "STRING", "REFERENCE"}; /* symbol type names */

mmap* states; /* states held as keys of mmap */

mmap* propositions; /* map from propositions to states */

mmap* edges; /* map from labels to edges (unledge) */

/* ---------------- stack for GC ---------------- */
/* At this stage GC is only used to hold string
   values created during intermediate ops on
   symbols. */
char** gc = NULL;
int gcfilledElements = 0;
int gcstsize = SCOPESTACKSIZE;

void gcClear() {
    while(--gcfilledElements >= 0) free(gc[gcfilledElements]);
    gcfilledElements = 0;
    }

void gcClearLazy() {
    static int clearCtr = 0;
    if((++clearCtr) % GCLAZINESS == 0) gcClear();
    }

/* Note - this is a result of fast coding, there's
   no need for GC. It was added to avoid memleaks
   in string concatenations (see symbolOperation). */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */


void init() {
    /* init string garbage collector */
    if(gc == NULL) gc = malloc(sizeof(char*) * gcstsize);

    /* prepare maps */
    if(states != NULL) freeMap(states, free);
    if(propositions != NULL) freeMap(propositions, clearNodeVectr);
    if(edges != NULL) freeMap(edges, clearUnledgeVectr);

    states = newMmap();
    propositions = newMmap();
    edges = newMmap();
    }


void cleanup() {
    /* clear formula trees */
    int i = 0;
    for(i = 0; i < compCtr; ++i) {
        freeTree(topTrees[i]);
        free(compnames[i]);
        }
    compCtr = 0;

    /* clear GC */
    gcClear();
    free(gc);
    gc = NULL;

    /* clear maps */
    if(states != NULL) freeMap(states, free);
    if(propositions != NULL) freeMap(propositions, clearNodeVectr);
    if(edges != NULL) freeMap(edges, clearUnledgeVectr);

    }


void evalModel(int modelno) {
    if(modelno < 0 || modelno >= compCtr)
        terror(-1, "No model with such number: %d\n", NULL, modelno);
    starteval(topTrees[modelno]);
    }


symbol* lookup(char* name, symbol** stable, int filledElements) {
    /* look for symbol */
    int insertPlace;
    for(insertPlace = 0; insertPlace < filledElements; ++insertPlace) {
        if(strcmp(stable[insertPlace]->name, name) == 0) break;
        }

    if(insertPlace == filledElements) return NULL; /* not found */
    else return stable[insertPlace]; /* found */
    }


symbol* newSymbol(char* name, int type, void* value) {

    symbol* ss = malloc(sizeof(symbol));
    if(!ss)  terror(-1, "Panic: Symbol alloc error!\n", NULL, 0);
    if(name != NULL) ss->name = name;
    else ss->name = NULL;
    ss->type = type;
    ss->value.chval = NULL;

    if(value != NULL) {
        switch(type) {
            case INTEGER:
                ss->value.intval = *((int*) value);
                break;
            case STRING:
            case REFERENCE:
                ss->value.chval = (char*) value;
                break;
            default:
                fprintf(stderr, "Unknown type!\n");
                free(ss);
                return NULL;
            }
        }

    return ss;
    }


symbol symbolOperation(symbol* s1, symbol* s2, char operation, int line) {

    /* INTEGER operations */
    if(s1->type == INTEGER && s2->type == INTEGER) {
        int result = 0;
        switch(operation) {
            case '+':
                result = s1->value.intval + s2->value.intval;
                break;
            case '-':
                result = s1->value.intval - s2->value.intval;
                break;
            case '*':
                result = s1->value.intval * s2->value.intval;
                break;
            case '/':
                if(s2->value.intval == 0) terror(line, "Error: division by zero!\n", NULL, 0);
                result = s1->value.intval / s2->value.intval;
                break;
            case '%':
                if(s2->value.intval == 0) terror(line, "Error: modulo zero!\n", NULL, 0);
                result = s1->value.intval % s2->value.intval;
                break;
            }

        symbol sym;
        sym.name = NULL;
        sym.type = INTEGER;
        sym.value.intval = result;
        return sym;
        }

    /* at least one symbol is a STRING */
    /* '+' is the only allowed operation for strings */
    if(operation != '+') terror(line, "Error: can only concatenate strings!\n", NULL, 0);
    char* leftSt = s1->value.chval;
    char* rightSt = s2->value.chval;
    if(s1->type == INTEGER) {
        char str[LONGESTCONC/2];
        sprintf(str, "%d", s1->value.intval);
        leftSt = str;
        }
    else if(s2->type == INTEGER) {
        char str[LONGESTCONC/2];
        sprintf(str, "%d", s2->value.intval);
        rightSt = str;
        }
    char* rslt = malloc(sizeof(char) * LONGESTCONC);
    sprintf(rslt, "%s%s", leftSt, rightSt);
    store(gc, rslt); /* hack to deal with bad design - keeping rstls here */

    symbol sym;
    sym.name = NULL;
    sym.type = STRING;
    sym.value.chval = rslt;

    return sym;
    }


int symbolComparison(symbol* s1, symbol* s2, char* relation, int line) {
    if(s1->type != INTEGER || s2->type != INTEGER)  /* can only compare INTEGERS */
        terror(line, "Bad relation %s (can only compare INTEGERS!)\n", relation, 0);

    if(strcmp(relation, "==") == 0) {
        return s1->value.intval == s2->value.intval;
        }
    else if(strcmp(relation, "!=") == 0) {
        return s1->value.intval != s2->value.intval;
        }
    else if(strcmp(relation, "<=") == 0) {
        return s1->value.intval <= s2->value.intval;
        }
    else if(strcmp(relation, ">=") == 0) {
        return s1->value.intval >= s2->value.intval;
        }
    else if(strcmp(relation, ">") == 0) {
        return s1->value.intval > s2->value.intval;
        }
    else if(strcmp(relation, "<") == 0) {
        return s1->value.intval < s2->value.intval;
        }

    /* bad relation symbol */
    terror(line, "Unknown relation %s\n", relation, 0);
    return 1;
    }


symbol* insert(char* name,
               int type,
               void* value,
               symbol** stable,
               int* filledElements,
               int stsize) {

    /* look for symbol */
    symbol* ss = lookup(name, stable, *filledElements);
    if(ss == NULL) { /* not found, make symbol */
        /* check for overflow */
        if(*filledElements >= stsize) {
            fprintf(stderr, "Panic: Symbol table full!\n");
            return NULL;
            }
        ss = malloc(sizeof(symbol));
        if(!ss) terror(-1, "Panic: Symbol alloc error!\n", NULL, 0);
        ss->name = strdup(name);
        ss->type = type;
        ss->value.chval = NULL;
        stable[*filledElements] = ss;
        ++(*filledElements);
        }
    else { /* found a symbol in table */
        /* check for type mismatch */
        if(type != ss->type) {
            fprintf(stderr, "Type mismatch: putting %s for %s in %s!\n", typenames[type], typenames[ss->type], ss->name);
            exit(1);
            }
        }

    /* insert/update */
    switch(type) {
        case INTEGER:
            ss->value.intval = *((int*) value);
            break;
        case STRING:
        case REFERENCE:
            if(ss->value.chval != NULL) free(ss->value.chval); /* free previous value */
            ss->value.chval = strdup(((char*) value));

            break;
        default:
            fprintf(stderr, "Unknown type!");
            free(ss);
            return NULL;
        }

    return ss;
    }


symbol* insertS(symbol* symb,
                symbol** stable,
                int* filledElements,
                int stsize) {

    if(symb->type == INTEGER)
        return insert(symb->name, symb->type, (void*) &symb->value.intval, stable, filledElements, stsize);
    if(symb->type == STRING)
        return insert(symb->name, symb->type, (void*) symb->value.chval, stable, filledElements, stsize);

    return NULL;
    }


void store(char** gc, char* str) {

    if(gcfilledElements >= gcstsize)
        terror(-1, "Bad GC stack full!\n", NULL, 0);

    gc[gcfilledElements++] = str;
    }


void freeSymbol(symbol* symb) {
    if(symb == NULL) return;

    free(symb->name);
    switch(symb->type) { /* for future reference */
        case STRING:
        case REFERENCE:
            free(symb->value.chval);
            break;
        case INTEGER:
            break;
        default:
            terror(-1, "Freeing unknown symbol %s!", symb->name, 0);
        }

    free(symb);
    }


void symbolTableClear(symbol** stable, int filledElements) {
    int i;
    for(i = 0; i < filledElements; ++i) {
        freeSymbol(stable[i]);
        }
    }


void displaySymbol(symbol* ss) {
    if(ss == NULL) {
        printf("NULL symbol\n");
        return;
        }

    printf("Symbol of name: %s, type: %s, ", ss->name, typenames[ss->type]);
    switch(ss->type) {
        case INTEGER:
            printf("value: %d\n", ss->value.intval);
            break;
        case STRING:
        case REFERENCE:
            printf("value: %s\n", ss->value.chval);
            break;
        default:
            terror(-1, "Display: unknown type!", NULL, 0);
        }

    }


void yyerror(char* s) {
    terror(yylineno, "Parse error: %s!\n", s, 0);
    }


void push(symbol** stable,
          int filledElements,
          int stsize,
          stableStack** top) {

    if(filledElements >= stsize) terror(-1, "Stack full!\n", NULL, 0);

    stableStack* ns = malloc(sizeof(stableStack));
    if(!ns) terror(-1, "Top alloc error!\n", NULL, 0);

    ns->stable = stable;
    ns->filledElements = filledElements;
    ns->stsize = stsize;
    ns->prev = *top;
    *top = ns;
    }


stableStack* pop(stableStack** top) {
    if(*top == NULL) return NULL;
    stableStack* rval = *top;
    *top = (*top)->prev;

    return rval;
    }


void freeStableStack(stableStack* stack) {
    if(stack == NULL) return;
    symbolTableClear(stack->stable, stack->filledElements);
    free(stack);
    }


symbol* insertToTop(char* name,
                    int type,
                    void* value,
                    stableStack* top
                   ) {

    stableStack* currStack = top;
    while(currStack != NULL) {
        symbol* st = lookup(name, currStack->stable, currStack->filledElements);
        if(st != NULL) return insert(name, type, value, currStack->stable, &currStack->filledElements, currStack->stsize);
        currStack = currStack->prev;
        }

    return insert(name, type, value, top->stable, &top->filledElements, top->stsize);
    }


symbol* insertToTopS(symbol* symb, stableStack* top) {
    if(symb->type == INTEGER)
        return insertToTop(symb->name, symb->type, (void*) &symb->value.intval, top);
    if(symb->type == STRING)
        return insertToTop(symb->name, symb->type, (void*) symb->value.chval, top);

    return NULL;
    }


symbol* lookupInStack(char* name, stableStack* top) {
    stableStack* currStack = top;
    while(currStack != NULL) {
        symbol* st = lookup(name, currStack->stable, currStack->filledElements);
        if(st != NULL) return st;
        currStack = currStack->prev;
        }

    return NULL;
    }


void displayStack(stableStack* top) {
    stableStack* currStack = top;
    int depth = 0;
    while(currStack != NULL) {
        int i;
        printf("-- stack depth: %d\n", depth--);
        for(i = 0; i < currStack->filledElements; ++i) displaySymbol((currStack->stable)[i]);
        currStack = currStack->prev;
        }
    }


astTree* newTreeNode(int type, int lineNumber, symbol* payload) {
    astTree* ntree = malloc(sizeof(astTree));
    ntree->operation = type;
    ntree->payload = payload;
    ntree->lineNumber = lineNumber;
    ntree->left = ntree->right = ntree->next = NULL;

    return ntree;
    }


void freeTree(astTree* tree) {
    if(tree == NULL) return;

    freeSymbol(tree->payload);
    freeTree(tree->left);
    freeTree(tree->right);
    freeTree(tree->next);
    free(tree);
    }


void debugTree(astTree* tree) {
    static int finish = 0;

    while(!finish) {
        printf("** Node type %s in line %d, payload: ", opnames[tree->operation - NEWSCOPE], tree->lineNumber);
        displaySymbol(tree->payload);
        printf(">> 0 - up, 1 - left [%d], 2 - right [%d], 3 - next [%d], q - end: ", (int)tree->left, (int)tree->right, (int)tree->next);

        char answ = getchar();
        getchar();
        switch(answ) {
            case '0':
                return;
                break;
            case '1':
                if(tree->left != NULL) debugTree(tree->left);
                else printf("No-go, it's NULL. Try again: \n");
                break;
            case '2':
                if(tree->right != NULL) debugTree(tree->right);
                else printf("No-go, it's NULL. Try again: \n");
                break;
            case '3':
                if(tree->next != NULL) debugTree(tree->next);
                else printf("No-go, it's NULL. Try again: \n");
                break;
            case 'q':
                finish = 1;
                break;
            default:
                printf("Pardon?");
            }
        }
    }


symbol exval(astTree* tree, stableStack* top) {

    /* terminals */
    if(tree->operation == TERMINAL) {
        if(tree->payload->type == INTEGER || tree->payload->type == STRING)
            return *(tree->payload);
        if(tree->payload->type == REFERENCE) {
            symbol* retval = lookupInStack(tree->payload->value.chval, top);
            if(retval == NULL) terror(tree->lineNumber, "Noninitialized variable %s dereferenced!\n", tree->payload->value.chval, 0);

            return *retval;
            }
        }

    /* expressions */
    symbol lfts;
    symbol rgts;
    if(tree->operation == EXPRESSION) {
        lfts = exval(tree->left, top);
        rgts = exval(tree->right, top);
        }
    else
        terror(tree->lineNumber, "Unexpected operation: %s!\n", opnames[tree->operation - NEWSCOPE], 0);

    return symbolOperation(&lfts, &rgts, *tree->payload->value.chval, tree->lineNumber);
    }


int cndval(astTree* tree, stableStack* top) {
    symbol lt = exval(tree->left, top);
    symbol rt = exval(tree->right, top);
    char* rel = tree->payload->value.chval;

    return symbolComparison(&lt, &rt, rel, tree->lineNumber);
    }


void eval(astTree* tree, stableStack* top) {

    /* open new scope */
    if(tree->operation == NEWSCOPE) {
        symbol* ntable[SCOPESTACKSIZE];
        push(ntable, 0, SCOPESTACKSIZE, &top);
        eval(tree->left, top);
        freeStableStack(pop(&top));
        }

    /* substitutions */
    if(tree->operation == DEFVAR) { /* define or update variable */
        char* name = (char*) tree->payload->value.chval;
        symbol expval = exval(tree->left, top); /* compute the expression value */
        expval.name = name;
        insertToTopS(&expval, top); /* update the reference */
        /* clear the string store - dodgy design!  */
        gcClearLazy();
        }

    /* flow control */
    /* --if */
    if(tree->operation == IFOP) {
        if(cndval(tree->left, top)) { /* IFOP */
            eval(tree->right->left, top);
            }
        else if(tree->right->right != NULL) { /* ELSE */
            eval(tree->right->right, top);
            }
        }
    /* --while */
    if(tree->operation == WHILEOP) {
        while(cndval(tree->left, top)) {
            eval(tree->right, top);
            }
        }
    /* --for */
    if(tree->operation == FOROP) {
        /* read range */
        astTree* rangeTree = tree->left;
        int start = rangeTree->left->payload->value.intval;
        int end = tree->left->right->payload->value.intval;

        astTree* mienhoTree = tree->right; /* body */

        char* varname = (char*) tree->payload->value.chval;;
        varname = (char*) tree->payload->value.chval;
        if(lookupInStack(varname, top) == NULL)
            terror(tree->lineNumber, "Noninitialized variable %s dereferenced!\n", tree->payload->value.chval, 0);

        int* i = &(lookupInStack(varname, top))->value.intval; /* direct access */
        for(*i = start; *i < end; ++(*i)) { /* actual FOR evaluation */
            eval(mienhoTree, top);
            }
        }

    /* commands */
    if(tree->operation == COMMAND) {
        const char* comdname = tree->payload->value.chval;

        if(strcmp(comdname, "STACKDMP") == 0) { /* --dump stack */
            displayStack(top);
            }

        else if(strcmp(comdname, "BLOOM") == 0) { /* --create graph node */
            symbol s = exval(tree->left, top);
            if(s.type == INTEGER) terror(tree->lineNumber, "Can't use integer as a node name!\n", NULL, 0);

            /* insert a pair new state -> empty vectr */
            mapInsert(states, strdup(s.value.chval), NULL, NULL, NULL);
            }

        else if(strcmp(comdname, "MARK_WITH") == 0) { /* --mark state with proposition */
            /* read state */
            symbol s = exval(tree->left, top);
            if(s.type == INTEGER) terror(tree->lineNumber, "Can't use integer as a node name!\n", NULL, 0);
            if(findIndex(states, s.value.chval) == -1) terror(tree->lineNumber, "State %s not defined!\n", s.value.chval, 0);

            /* read proposition */
            symbol prop = exval(tree->right, top);
            if(prop.type == INTEGER) terror(tree->lineNumber, "Can't use integer as a label!\n", NULL, 0);
            mapInsert(propositions, strdup(prop.value.chval), strdup(s.value.chval),
                      ( int (*)(void* v1, void* v2) ) strcmp, free);
            }

        else if(strcmp(comdname, "JOIN_WITH") == 0) { /* --join states with label */
            /* read states */
            astTree* edgeTr = tree->left;

            symbol sorigin = exval(edgeTr->left, top);
            symbol sgoal = exval(edgeTr->right, top);
            if(sorigin.type == INTEGER || sgoal.type == INTEGER)
                terror(tree->lineNumber, "Can't use integer as a node name!\n", NULL, 0);
            if(findIndex(states, sorigin.value.chval) == -1)
                terror(tree->lineNumber, "State %s not defined!\n", sorigin.value.chval, 0);
            if(findIndex(states, sgoal.value.chval) == -1)
                terror(tree->lineNumber, "State %s not defined!\n", sgoal.value.chval, 0);

            /* read label/proposition */
            symbol prop = exval(tree->right, top);
            if(prop.type == INTEGER) terror(tree->lineNumber, "Can't use integer as a label!\n", NULL, 0);

            mapInsert(edges, strdup(prop.value.chval), newUnledge(strdup(sorigin.value.chval), strdup(sgoal.value.chval)),
                      (int (*)(void* v1, void* v2)) unledgeEq, freeUnledge);
            }

        else terror(tree->lineNumber, "Unknown command %s called!\n", comdname, 0);

        }

    /* sequential call */
    if(tree->next != NULL) eval(tree->next, top);

    }


void starteval(astTree* tree) {
    stableStack* top = NULL;
    symbol* ntable[SCOPESTACKSIZE];
    push(ntable, 0, SCOPESTACKSIZE, &top);
    eval(tree, top);
    freeStableStack(pop(&top));
    }


formTree* newFormTree(int pathSel, int oper,
                      char* varbName, vectr* elts) {
    formTree* nft = (formTree*) malloc(sizeof(formTree));
    const char* erreason = NULL;
    if(pathSel < EXISTS || pathSel > NONEPARA) erreason = "selector";
    if(oper < MNEXT || oper > MNONE) erreason = "operator";
    if(erreason != NULL) terror(-1, "Wrong %s type!\n", erreason, 0);

    nft->path_selector = pathSel;
    nft->opertr = oper;
    nft->varName = NULL;
    nft->elements = NULL;
    nft->left = NULL;
    nft->right = NULL;

    switch(pathSel) {
        case EXISTS:
        case ALWAYS:
            nft->elements = elts;
            break;
        case EXISTSPARA:
        case ALWAYSPARA:
        case NONEPARA:
            nft->varName = varbName;
            break;
        }

    return nft;
    }


void freeFormTree(formTree* ftree) {
    if(ftree == NULL) return;
    if(ftree->varName != NULL) free(ftree->varName);
    if(ftree->elements != NULL) freeVectr(ftree->elements, free);
    free(ftree->left);
    free(ftree->right);
    free(ftree);
    }


void displayFormTree(formTree* ftree) {
    while(1) {
        if(ftree == NULL) {
            printf("Null formula, going up!\n");
            return;
            }

        if(ftree->path_selector == EXISTS || ftree->path_selector == ALWAYS ||
                ftree->path_selector == EXISTSPARA || ftree->path_selector == ALWAYSPARA) { //modals

            if(ftree->path_selector == EXISTS || ftree->path_selector == EXISTSPARA) printf("E");
            else printf("A");

            switch(ftree->opertr) {
                case MNEXT:
                    printf("X( ");
                    break;
                case MGLOBALLY:
                    printf("G( ");
                    break;
                case MFUTURE:
                    printf("F( ");
                    break;
                case MGLOBALLYOM:
                    printf("Gom( ");
                    break;
                case MUNTIL:
                    printf("U( ");
                    break;
                default:
                    terror(-1, "Modal: wrong operator here: %d\n", NULL, ftree->opertr);
                }

            if(ftree->path_selector == EXISTS || ftree->path_selector == ALWAYS) {
                int i = 0;
                for(i = 0; i < (ftree->elements)->elements; ++i)
                    printf("%s ", (char*) get(ftree->elements, i));
                }
            else printf("$%s ", ftree->varName);

            }
        else if(ftree->path_selector == NONEPARA && (ftree->opertr == BNOT || ftree->opertr == BAND
                || ftree->opertr == BOR || ftree->opertr == MNONE)) { //booleans

            switch(ftree->opertr) {
                case BNOT:
                    printf("NOT(");
                    break;
                case BAND:
                    printf("AND(");
                    break;
                case BOR:
                    printf("OR(");
                    break;
                case MNONE:
                    printf("(%s",ftree->varName);
                    break;
                default:
                    terror(-1, "Boolean: wrong operator here: %d\n", NULL, ftree->path_selector);
                }

            }
        else terror(-1, "Wrong selector here: %d\n", NULL, ftree->path_selector);
        printf(")\n");

        printf("Select 0-up, 1-left, 2-right, q for quit\n");
        unsigned char answ = getchar();
        getchar();
        if(answ == '0') return;
        else if(answ == '1') displayFormTree(ftree->left);
        else if(answ == '2') displayFormTree(ftree->right);
        else exit(0);
        }

    }

