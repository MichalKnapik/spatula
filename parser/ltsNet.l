/**
*
* Copyright (c) 2013-... Michal Knapik, ICS PAS
* Distributed under the GNU GPL v2. For full terms see the COPYING.txt file.
*
*/

%option noyywrap yylineno
%x comment

%{
#include "ast.h"
#include "ltsNet.tab.h"
%}

/* abbrev defns */
STRING ([a-zA-Z\@\_]+[a-zA-Z0-9\@\_]*)
INTEGER ([-]?[0-9]+)
WSPACE ([ \t\n]*)

/* reserved words */
IF "if"
ELSE "else"
FOR "for"
WHILE "while"
IN "in"
SYSTEM "module"
FORRNG ".."
STACKDMP "stackdump"
SEMANTICS "semantics"
DISJUNCTIVE "disjunctive"
BLOOM "bloom"
MARK_WITH "mark_with"
JOIN_WITH "join_with"
VERIFY "verify"
LIMITS "switches"

AND "and"
OR "or"
NOT "not"

EXISTSNEXT "#EX"
ALLNEXT "#AX"
EXISTSFUTURE "#EF"
ALLFUTURE "#AF"
EXISTSGLOB "#EG"
ALLGLOB "#AG"
EXISTSGLOBOM "#EGom"
ALLGLOBOM "#AGom"
EXISTSUNTIL "#EU"
ALLUNTIL "#AU"
VARSEL "$"

/* lexer rules */
%%

[ \t\n\r]+ { }

{SYSTEM} { return SYSTEM; }
{VERIFY} { return VERIFY; }
{LIMITS} { return LIMITS; }
{SEMANTICS} { return SEMANTICS; }
{DISJUNCTIVE} { return DISJUNCTIVE; }

{IF} { return IF; }
{ELSE} { return ELSE; }
{FOR} { return FOR; }
{WHILE} { return WHILE; }
{IN} { return IN; }
{FORRNG} { return FORRNG; }

{STACKDMP} { return STACKDMP; }
{BLOOM} { return BLOOM; }
{MARK_WITH} { return MARK_WITH; }
{JOIN_WITH} { return JOIN_WITH; }

{AND} { return AND; }
{OR} { return OR; }
{NOT} { return NOT; }

{EXISTSNEXT} { return EXISTSNEXT; }
{ALLNEXT} { return ALLNEXT; }
{EXISTSFUTURE} { return EXISTSFUTURE; }
{ALLFUTURE} { return ALLFUTURE; }
{EXISTSGLOB} { return EXISTSGLOB; }
{ALLGLOB} { return ALLGLOB; }
{EXISTSGLOBOM} { return EXISTSGLOBOM; }
{ALLGLOBOM} { return ALLGLOBOM; }
{EXISTSUNTIL} { return EXISTSUNTIL; }
{ALLUNTIL} { return ALLUNTIL; }

{VARSEL} { return VARSEL; }

"{" { return OPENBODY; }
"}" { return CLOSEBODY; }
"(" { return OPENBR; }
")" { return CLOSEBR; }
"[" { return OPENSQBR; }
"]" { return CLOSESQBR; }


"=" { return SUBST; }

"+" { 
  yylval.stringval = strdup(yytext); 
  return PLUS; 
}
"*" { 
  yylval.stringval = strdup(yytext); 
  return TIMES; 
}
"-" { 
  yylval.stringval = strdup(yytext); 
  return MINUS; 
}
"/" { 
  yylval.stringval = strdup(yytext); 
  return DIV; 
}
"%" { 
  yylval.stringval = strdup(yytext); 
  return MOD; 
}

("<"|"<="|"=="|">="|">"|"!=") {
  yylval.stringval = strdup(yytext); 
  return REL; 
}

\"([ \t]*{STRING}[ \t]*)*\"|\"[ \t]*\" { 
  char locstr[LONGESTCONC] = { '\0' };
  memcpy(locstr, yytext + 1, strlen(yytext) - 2);
  yylval.stringval = strdup(locstr);
  return STRINGVAL; 
}

{STRING} { 
  yylval.stringval = strdup(yytext); 
  return REFERENCEVAL; 
}

{INTEGER} { 
  yylval.intval = atoi(yytext); 
  return INTEGERVAL; 
}

";" { return EOS; }
"," { return CMM; }

"/*" BEGIN(comment);
<comment>[^*\n]*        /* From flex man: eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             /* ignore newlines */
<comment>"*"+"/"        BEGIN(INITIAL);

. {}

%%
