/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vectr.h"

    /* the maximal number of components */
#define COMPONENTSMAXNO 1000

    /* the size of a stack of operands within a single scope */
#define SCOPESTACKSIZE 1000

    /* the longest string allowed in concatenations */
#define LONGESTCONC 100

    /* how often gc gets called */
#define GCLAZINESS 100

    extern int yylineno;

    void yyerror(char* s);

    /* Clears up string GC */
    void gcClear();

    /* Clears up string GC on ech consecutive GCLAZINESS number of steps */
    void gcClearLazy();

    /* Sets up propmap, stacks, etc. Run before anything else. Also:
       run after loading and evaluating a module and collecting results, before
       loading another. */
    void init();

    /* Clears and releases memory */
    void cleanup();

    /* Evaluates modelno-th model. After this you should collect model details from
       globals and cleanup. */
    void evalModel(int modelno);

    /*@@@@@@@@@@@@@@ SYMBOLS, LOOKUPS AND LOCAL VARIABLES @@@@@@@@@@@@@@*/

    /* Symbol types allowed. REFERENCE is a variable name. */
    enum types {INTEGER = 0, STRING, REFERENCE};

    /* Typical usage when creating new vars within scopes

       Provide global pointer to the current stack of variables:
       struct stableStack* top = NULL;

       When entering new scope:
       symbol* ss[100];
       push(ss, 0, SCOPESTACKSIZE, &top);
       insertToTop(..., top); insert or update variable in current scope
       Cleanup when leaving the scope:
       freeStableStack(pop(&top)); */

    /* Symbols and symbol table defns */

    /* Symbol */
    typedef struct {
        char* name;
        int type;
        union {
            int intval;
            char* chval;
            } value;
        } symbol;

    symbol* newSymbol(char* name, int type, void* value);

    void displaySymbol(symbol* ss);

    void freeSymbol(symbol* symb);

    /* Operations on symbols:
       INTEGER + INTEGER -> INTEGER
       and other cases yield strings via concatenation */
    symbol symbolOperation(symbol* s1, symbol* s2, char operation, int line);

    /* Comparison of two symbols. Works only for INTEGERS. */
    int symbolComparison(symbol* s1, symbol* s2, char* relation, int line);

    /* Find a symbol in table and return it, NULL if not found */
    symbol* lookup(char* name, symbol** stable, int filledElements);

    /* Lookup a symbol in table and change it if found or create and insert if not.
       Input value will be cast to appropriate type, filledElements is updated.
       Return NULL if table full or on error (e.g. type change). Makes a copy!      */
    symbol* insert(char* name,
                   int type,
                   void* value,
                   symbol** stable,
                   int* filledElements,
                   int stsize);

    /* A version of insert. */
    symbol* insertS(symbol* symb,
                    symbol** stable,
                    int* filledElements,
                    int stsize);

    /* Stores string in gc */
    void store(char** gc, char* str);

    /* Symbol table cleanup */
    void symbolTableClear(symbol** stable, int filledElements);

    /* Stack of symbol tables */
    struct ststack {
        symbol** stable; /* current table */
        int filledElements;
        int stsize;
        struct ststack* prev; /* previous table */
        };
    typedef struct ststack stableStack;

    void displayStack(stableStack* top);

    /* Push a new symbol table on a stack. Moves top stack pointer to new position,
       so if you have:
       stableStack* top;
       to hold your pointer, then call push(..., &top); */
    void push(symbol** stable,
              int filledElements,
              int stsize,
              stableStack** top);

    /* Pop the current symbol table from stack. The top is moved to the
       previous entry, so if you have:
       stableStack* top;
       to hold your pointer, then call pop(&top); */
    stableStack* pop(stableStack** top);

    /* Cleanup of a single stableStack (.prev not touched).
       Intended use: freeStableStack(pop(&top));
       should clean the symbol stack at the var scope end
       and move the top pointer to the previous entry. */
    void freeStableStack(stableStack* stack);

    /* --- Stack interface to symbol tables --- */

    /* Find a symbol in whole stack and return it, NULL if not found.
       Starts from a top stack element and goes backward until a find
       or full stack is explored. */
    symbol* lookupInStack(char* name, stableStack* top);

    /* Call insert on stack's symbol table.
       Updates symbol in current or encompassing scopes,
       inserts to the current stack only. */
    symbol* insertToTop(char* name,
                        int type,
                        void* value,
                        stableStack* top
                       );

    /* A version of insertToTop. */
    symbol* insertToTopS(symbol* symb, stableStack* top);

    /* @@@@@@@@@@@@@@ AST TREES @@@@@@@@@@@@@@ */

    /* Named operation types:
       NEWSCOPE   - dummy, used to start new scope for vars
       DEFVAR (=) - define or update variable in a given scope
       EXPRESSION - evaluates to integer or string
       RELATION   - evaluates to integer or string
       TERMINAL   - integer or string in payload
       IFOP, IFBRNCH, FOROP, WHILEOP - flow expressions
       COMMAND    - used to call special commands
       DUMMY      - used e.g. in for range defn */

    enum optype {NEWSCOPE = 300, DEFVAR, TERMINAL, EXPRESSION, RELATION,
                 IFOP, IFBRNCH, FOROP, WHILEOP, COMMAND, DUMMY
                };

    /* The parsed program is saved as an AST tree.
       Payload is either a number or a cstring. */
    struct asttr {
        int operation; /* operation type */
        int lineNumber;
        symbol* payload;
        struct asttr* left;   /* left operand */
        struct asttr* right;  /* right operand */
        struct asttr* next;   /* next instruction */
        };
    typedef struct asttr astTree;

    astTree* newTreeNode(int type, int lineNumber, symbol* payload);

    /* Frees the ast tree. */
    void freeTree(astTree* tree);

    /* Interactive tree crawl. */
    void debugTree(astTree* tree);

    /* The evaluation function. Call this on top of the formula/program tree, to
       recursively evaluate the whole tree, i.e., to run the formula/program.
       Maintains internal symbol stacks starting from top. */
    void eval(astTree* tree, stableStack* top);

    /* First eval call */
    void starteval(astTree* tree);

    /* Evaluates expression in tree. Returns the value of the expression */
    symbol exval(astTree* tree, stableStack* top);

    /* Evaluates condition in tree, returns true/false */
    int cndval(astTree* tree, stableStack* top);

    /* @@@@@@@@@@@@@@ FORMULA TREES @@@@@@@@@@@@@@ */

    enum pathselectors {EXISTS, ALWAYS, EXISTSPARA, ALWAYSPARA, NONEPARA};
    enum operators {MNEXT, MGLOBALLY, MGLOBALLYOM, MUNTIL, MFUTURE, BNOT, BAND, BOR, MNONE};

    /* This tree holds a pmARCTL formula */
    struct formtr {
        int path_selector; //pathselectors
        int opertr; //operators
        char* varName; //variable name if the formula is parametric modal or proposition name if it is boolean terminal
        vectr* elements; //vector of actions if the formula is nonparametric modal

        struct formtr* left;
        struct formtr* right;
        };
    typedef struct formtr formTree;

    /* Returns a new formtr. Use this rather than 'new' */
    formTree* newFormTree(int pathSel, int oper,
                          char* varbName, vectr* elts);

    /* Recursively frees space for the formTree and its subtrees */
    void freeFormTree(formTree* ftree);

    void displayFormTree(formTree* ftree);

#ifdef __cplusplus
    }
#endif

#endif
