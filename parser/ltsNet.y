%{
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
#include <stdlib.h>
#include <string.h>

/* from ast.c: */
extern astTree* topTrees[COMPONENTSMAXNO]; /* used to hold the formula trees */
extern char* compnames[COMPONENTSMAXNO]; /* the names of the components */
extern formTree* formula; /* the formula to be verified */
extern vectr* limits; /* the actions allowed in synthesised valuations */
extern int compCtr; /* counts the number of components */
extern int semantics; /* the way the global states are marked */
%}

/* token declarations */
%union {
  astTree* astree;
  formTree* formtree;
  char* stringval;
  vectr* vectorv;
  int intval;
}

%left PLUS MINUS
%left TIMES DIV
%left MOD
%left CMM
%right NOT
%left AND
%left OR
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <astree> statement
%type <astree> command
%type <astree> statementlist
%type <astree> expression
%type <astree> condition
%type <astree> terminal
%type <astree> roots
%type <stringval> varselect

%type <formtree> formulav
%type <formtree> finform
%type <formtree> boolform
%type <formtree> subform
%type <vectorv> actset

%token OPENBODY
%token CLOSEBODY
%token OPENBR
%token CLOSEBR
%token OPENSQBR
%token CLOSESQBR

%token IF
%token ELSE
%token FOR 
%token WHILE
%token IN
%token FORRNG

%token STACKDMP
%token BLOOM
%token MARK_WITH
%token JOIN_WITH

%token AND
%token OR
%token NOT

%token EXISTSNEXT 
%token ALLNEXT 
%token EXISTSFUTURE
%token ALLFUTURE
%token EXISTSGLOB 
%token ALLGLOB 
%token EXISTSGLOBOM 
%token ALLGLOBOM 
%token EXISTSUNTIL 
%token ALLUNTIL 
%token VARSEL

%token <stringval> PLUS
%token <stringval> TIMES
%token <stringval> MINUS
%token <stringval> DIV
%token <stringval> MOD
%token <stringval> REL
%token <stringval> REFERENCEVAL
%token <stringval> STRINGVAL
%token <intval> INTEGERVAL

%token SYSTEM
%token SEMANTICS
%token DISJUNCTIVE
%token SUBST
%token EOS
%token CMM

%token VERIFY
%token LIMITS

%start code

/* grammar rules */
%%

code: semselect roots VERIFY formulav limitsel {}

limitsel: LIMITS actset {
  limits = $2;
 }
|
;

semselect: SEMANTICS DISJUNCTIVE EOS { semantics = 1; }
| { semantics = 0; };

roots: SYSTEM REFERENCEVAL statementlist roots {
  if(compCtr > COMPONENTSMAXNO) terror(-1, "Too many components!", NULL, 0);
  topTrees[compCtr] = $3;
  compnames[compCtr] = $2;

  ++compCtr;
 }
| SYSTEM REFERENCEVAL statementlist {
  if(compCtr > COMPONENTSMAXNO) terror(-1, "Too many components!", NULL, 0);
  topTrees[compCtr] = $3;
  compnames[compCtr] = $2;

  ++compCtr;
 };

statementlist: statement statementlist {
  astTree* ast = $1;
  ast->next = $2;

  $$ = ast;
 }
| statement { $$ = $1; }
;

statement: OPENBODY statementlist CLOSEBODY {
  astTree* ast = newTreeNode(NEWSCOPE, yylineno, NULL);
  ast->left = $2;

  $$ = ast;
 }
| REFERENCEVAL SUBST expression EOS { 
  /* reference variable is kept in value */
  astTree* ast = newTreeNode(DEFVAR, yylineno, newSymbol(NULL, REFERENCE, $1));  
  ast->left = $3;

  $$ = ast;
 }
| IF OPENBR condition CLOSEBR statement %prec LOWER_THAN_ELSE {
  /* condition kept in left node, right node contains treeNode with: */
  /* positive statement in left and NULL in right */
  astTree* ast = newTreeNode(IFOP, yylineno, NULL);  
  ast->left = $3;
  astTree* els = newTreeNode(IFBRNCH, yylineno, NULL);
  els->left = $5;
  ast->right = els;

  $$ = ast;  
  }
| IF OPENBR condition CLOSEBR statement ELSE statement {
  /* condition kept in left node, right node contains treeNode with: */
  /* positive statement in left and else in right */
  astTree* ast = newTreeNode(IFOP, yylineno, NULL);  
  ast->left = $3;
  astTree* els = newTreeNode(IFBRNCH, yylineno, NULL);
  els->left = $5;
  els->right = $7;
  ast->right = els;

  $$ = ast;  
 } 
| WHILE OPENBR condition CLOSEBR statement {
  /* condition kept in left node, right node contains statement */
  astTree* ast = newTreeNode(WHILEOP, yylineno, NULL);  
  ast->left = $3;
  ast->right = $5;

  $$ = ast;  
 }
| FOR REFERENCEVAL IN OPENBR INTEGERVAL FORRNG INTEGERVAL CLOSEBR statement {
  /* variable name is kept in the payload value */
  astTree* ast = newTreeNode(FOROP, yylineno, newSymbol(NULL, REFERENCE, $2));

  /* range is kept in the left node of ast */
  astTree* rng = newTreeNode(DUMMY, yylineno, NULL);  
  ast->left = rng;

  /* lower number is in left subnode of range, right in right */
  /* each integer value is kept in the payload value */
  astTree* lowerrng = newTreeNode(TERMINAL, yylineno, newSymbol(NULL, INTEGER, &$5));  
  astTree* higherrng = newTreeNode(TERMINAL, yylineno, newSymbol(NULL, INTEGER, &$7));
  rng->left = lowerrng;
  rng->right = higherrng;

  /* statement is held in right node of ast */
  ast->right = $9;

  $$ = ast;
 }
| command { $$ = $1;}
;

command: STACKDMP EOS {
  astTree* ast = newTreeNode(COMMAND, yylineno, newSymbol(NULL, STRING, strdup("STACKDMP")));
  $$ = ast;
 }
| BLOOM OPENBR terminal CLOSEBR EOS {
  astTree* ast = newTreeNode(COMMAND, yylineno, newSymbol(NULL, STRING, strdup("BLOOM")));
  ast->left = $3;
  $$ = ast;  
 }
| MARK_WITH OPENBR terminal CMM terminal CLOSEBR EOS {
  /* $3 is a state, $5 is a proposition */
  astTree* ast = newTreeNode(COMMAND, yylineno, newSymbol(NULL, STRING, strdup("MARK_WITH")));
  ast->left = $3;
  ast->right = $5;

  $$ = ast;    
 }
| JOIN_WITH OPENBR terminal CMM terminal CMM terminal CLOSEBR EOS {
    /* $3 is an origin state, $5 is a goal state, $7 is a label */
    astTree* ast = newTreeNode(COMMAND, yylineno, newSymbol(NULL, STRING, strdup("JOIN_WITH")));

    /* states are kept in the left node */
    astTree* sts = newTreeNode(DUMMY, yylineno, NULL);
    sts->left = $3;
    sts->right = $5;
    ast->left = sts;

    /* label is kept in the right node */
    ast->right = $7;
    $$ = ast;
    }
;

expression: expression PLUS expression { 
  /* operation symbol is kept in the payload value */
  astTree* ast = newTreeNode(EXPRESSION, yylineno, newSymbol(NULL, STRING, $2));    
  ast->left = $1;
  ast->right = $3;

  $$ = ast;
 }
| expression TIMES expression { 
  /* operation symbol is kept in the payload value */
  astTree* ast = newTreeNode(EXPRESSION, yylineno, newSymbol(NULL, STRING, $2));    
  ast->left = $1;
  ast->right = $3;

  $$ = ast;
 }
| expression MINUS expression { 
  /* operation symbol is kept in the payload value */
  astTree* ast = newTreeNode(EXPRESSION, yylineno, newSymbol(NULL, STRING, $2));    
  ast->left = $1;
  ast->right = $3;

  $$ = ast;
 }
| expression DIV expression { 
  /* operation symbol is kept in the payload value */
  astTree* ast = newTreeNode(EXPRESSION, yylineno, newSymbol(NULL, STRING, $2));    
  ast->left = $1;
  ast->right = $3;

  $$ = ast;
 }
| expression MOD expression { 
  /* operation symbol is kept in the payload value */
  astTree* ast = newTreeNode(EXPRESSION, yylineno, newSymbol(NULL, STRING, $2));    
  ast->left = $1;
  ast->right = $3;

  $$ = ast;
 }
| OPENBR expression CLOSEBR { $$ = $2; }
| terminal { $$ = $1; }
;

terminal: INTEGERVAL {
  /* integer value is kept in the payload value */
  astTree* ast = newTreeNode(TERMINAL, yylineno, newSymbol(NULL, INTEGER, &$1));  
  $$ = ast;
 }
| STRINGVAL {
  /* string value is kept in the payload value */
  astTree* ast = newTreeNode(TERMINAL, yylineno, newSymbol(NULL, STRING, $1));  
  $$ = ast;
  }
| REFERENCEVAL {
  /* reference value is kept in the payload value */
  astTree* ast = newTreeNode(TERMINAL, yylineno, newSymbol(NULL, REFERENCE, $1));  
  $$ = ast;
  }

condition: expression REL expression {
  /* relation symbol is kept in the payload value */
  astTree* ast = newTreeNode(RELATION, yylineno, newSymbol(NULL, STRING, $2));  
  ast->left = $1;
  ast->right = $3;
  $$ = ast;  
 }

formulav: boolform EOS {
  formula = $1;
  $$ = $1;
}

boolform: boolform OR subform {
  formTree* frmla = newFormTree(NONEPARA, BOR, NULL, NULL);
  frmla->left = $1;
  frmla->right = $3;
  $$ = frmla;
}
| subform {
  $$ = $1;
}

subform: subform AND finform {
  formTree* frmla = newFormTree(NONEPARA, BAND, NULL, NULL);
  frmla->left = $1;
  frmla->right = $3;
  $$ = frmla;
}
| finform {
  $$ = $1;
}

finform: EXISTSNEXT OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTS, MNEXT, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| ALLNEXT OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYS, MNEXT, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSFUTURE OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTS, MFUTURE, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| ALLFUTURE OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYS, MFUTURE, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSGLOB OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTS, MGLOBALLY, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| ALLGLOB OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYS, MGLOBALLY, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSGLOBOM OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTS, MGLOBALLYOM, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| ALLGLOBOM OPENBR actset finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYS, MGLOBALLYOM, NULL, $3);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSUNTIL OPENBR actset finform CMM finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTS, MUNTIL, NULL, $3);
  frmla->left = $4;
  frmla->right = $6;
  $$ = frmla;
}
| ALLUNTIL OPENBR actset finform CMM finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYS, MUNTIL, NULL, $3);
  frmla->left = $4;
  frmla->right = $6;
  $$ = frmla;
}
| REFERENCEVAL {
  formTree* frmla = newFormTree(NONEPARA, MNONE, $1, NULL);
  $$ = frmla;
}
| EXISTSNEXT OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTSPARA, MNEXT, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| ALLNEXT OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYSPARA, MNEXT, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSFUTURE OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTSPARA, MFUTURE, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| ALLFUTURE OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYSPARA, MFUTURE, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSGLOB OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTSPARA, MGLOBALLY, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| ALLGLOB OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYSPARA, MGLOBALLY, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSGLOBOM OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTSPARA, MGLOBALLYOM, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| ALLGLOBOM OPENBR varselect finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYSPARA, MGLOBALLYOM, $3, NULL);
  frmla->right = $4;
  $$ = frmla;
}
| EXISTSUNTIL OPENBR varselect finform CMM finform CLOSEBR {
  formTree* frmla = newFormTree(EXISTSPARA, MUNTIL, $3, NULL);
  frmla->left = $4;
  frmla->right = $6;
  $$ = frmla;
}
| ALLUNTIL OPENBR varselect finform CMM finform CLOSEBR {
  formTree* frmla = newFormTree(ALWAYSPARA, MUNTIL, $3, NULL);
  frmla->left = $4;
  frmla->right = $6;
  $$ = frmla;
}
| OPENBR boolform CLOSEBR {
  $$ = $2;
}
| NOT finform {
  formTree* frmla = newFormTree(NONEPARA, BNOT, NULL, NULL);
  frmla->right = $2;
  $$ = frmla;
}

varselect: VARSEL REFERENCEVAL EOS {
  $$ = $2;
}

actset: REFERENCEVAL CMM actset {
  append($3, $1);
  $$ = $3;
}
| REFERENCEVAL EOS {
  vectr* v = newVectr();
  append(v, $1);
  $$ = v;
}

%%
