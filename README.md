SPATULA alpha
========================================================

SPATULA is a simple parametric synthesis tool prototype. It takes as an input a network of graphs 
(also called mixed transition systems) with labeled nodes and transitions and a parametric 
branching-action logic property to be verified. The output consists of a set of substitutions
under which the property holds. 

Installation
------------
It should be easy on a recent Linux machine, just type 'make'. 
The main dependencies are induced by Fabio Somenzi's CUDD package (here, the CUDD 2.5.0 is used).


Model description language
--------------------------

The used language is a simple C-like one. A system consists of one or more independent 
modules. A module is defined using the "module" keyword, as in:

    module Single:
      i = 0;
      for i in (0..100) {
        vertex = "vertex_" + i;
        bloom(vertex);
      }

(The above code creates 100 vertices, named "vertex_n" where n ranges over [0, 99].
The range in for loop needs to be literal, without references.)

All the used variables need to be defined and initialised. There is no variable
shadowing, so once defined a variable is visible in its scope and all subscopes.
Variables can only take two types, string and integer. Arithmetic operations on 
integers are allowed; strings implement only concatenation. An integer can be added 
to string and the result is a string via concatenation. Once a variable has an 
associated value, its type can't be changed.

    module Variables:
    /* valid statements */
    st = "a string";
    st = st + "_";  
    st = st + 1001; 
    in = 1234;
    nst = st + in;
    inval = in/0; /* causes runtime error */
    
    /* errors */
    st = 123; /* type mismatch */
    in = st; /* type mismatch */

Core graph-building functionalities are:

    /* emerges a vertex with vertex_name */
    bloom(vertex_name); 
    
    /* labels previously emerged vertex vertex_name with proposition */
    mark_with(vertex_name, proposition); 

Note: if you mark a vertex with "initial" label, then it will be an initial node.

    /* joins first vertex with second one (both need to be bloomed before)
    and labels the transition with trans_label                          */
    join_with(first, last, trans_label);

The language supports three types of flow control operations: while, if/else and for.
The example of "for" use is shown above. The remaining operations are as in the following
example.

    module Single:
      x = 0; max = 5;
      while(x < max) {

        current = "vertex_" + x; /* current vertex name */
        bloom(current); /* make vertex */

        if(x >= 1) { /* join previous and current vertices with jump label */
        previous = "vertex_" + (x - 1);
        join_with(previous, current, "jump"); 
        }

      if(x%2 == 1) /* mark vertices with labels */
        mark_with(current, "p");
      else 
        mark_with(current, "q");

      x = x + 1;    
     }       

     /* join last with first */
     first = "vertex_" + (max - 1);
     last = "vertex_" + 0;
     join_with(first, last, "return");

Only simple comparisons are allowed in conditions, so use double 'if' in place
of 'and', e.g., 

     if(i != 2) if(i != 3) mark_with(vec, "p");

For simple debugging the "stackdump" operation can be called to print the current interpreter stack.

If you wish to use a disjunctive semantics for parallel composition, put:

    semantics disjunctive;

before all module definitions.

Formula language
----------------

There are two types of operators: boolean and modal; the modal operators are divided into parametric and nonparametric.
All the natural binding rules are followed; formulae can be put in parentheses to affect the precedence, as usually.
In what follows f and g are valid formulae.

#### Boolean operators:
 * f and g
 * f or g
 * not f

#### Modal operators:
 * #EX(selector; (f))
 * #AX(selector; (f))
 * #EG(selector; (f))
 * #AG(selector; (f))
 * #EF(selector; (f))
 * #AF(selector; (f))
 * #EGom(selector; (f))
 * #AGom(selector; (f))
 * #EU(selector; (f), (g))
 * #AU(selector; (f), (g))

The selector in the modal formulae can be either a set of known actions separated by comma or a variable started with 
$ (dollar symbol), e.g.: 

     #EG( act1, act2, act3; proposition1 ) and #EG( $variable1 ; proposition1 );

A complete formula should end with semicolon.

The set of actions allowed to be turned off in the synthesised valuations can be limited using an additional clause at the end of the file, below the formula, e.g.:

    switches:
    left, right, forward;

The above says that we consider valuations in which left, right
or forward may not be present (they can't all be missing simultaneously).


