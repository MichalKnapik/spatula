/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_LTSNET_TAB_H_INCLUDED
# define YY_YY_LTSNET_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PLUS = 258,
    MINUS = 259,
    TIMES = 260,
    DIV = 261,
    MOD = 262,
    CMM = 263,
    NOT = 264,
    AND = 265,
    OR = 266,
    LOWER_THAN_ELSE = 267,
    ELSE = 268,
    OPENBODY = 269,
    CLOSEBODY = 270,
    OPENBR = 271,
    CLOSEBR = 272,
    OPENSQBR = 273,
    CLOSESQBR = 274,
    IF = 275,
    FOR = 276,
    WHILE = 277,
    IN = 278,
    FORRNG = 279,
    STACKDMP = 280,
    BLOOM = 281,
    MARK_WITH = 282,
    JOIN_WITH = 283,
    EXISTSNEXT = 284,
    ALLNEXT = 285,
    EXISTSFUTURE = 286,
    ALLFUTURE = 287,
    EXISTSGLOB = 288,
    ALLGLOB = 289,
    EXISTSGLOBOM = 290,
    ALLGLOBOM = 291,
    EXISTSUNTIL = 292,
    ALLUNTIL = 293,
    VARSEL = 294,
    REL = 295,
    REFERENCEVAL = 296,
    STRINGVAL = 297,
    INTEGERVAL = 298,
    SYSTEM = 299,
    SEMANTICS = 300,
    DISJUNCTIVE = 301,
    SUBST = 302,
    EOS = 303,
    VERIFY = 304,
    LIMITS = 305
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 27 "ltsNet.y" /* yacc.c:1909  */

  astTree* astree;
  formTree* formtree;
  char* stringval;
  vectr* vectorv;
  int intval;

#line 113 "ltsNet.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_LTSNET_TAB_H_INCLUDED  */
