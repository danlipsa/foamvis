%{

#include <string.h>
#include <stdio.h>
#include "lexYacc.h"

int yylex(void);
void yyerror (char const *);

%}
// directives
%defines
%token-table 
%error-verbose
%expect 4
%locations

 // KEYWORDS  -- should come before other tokens --
%token PARAMETER "PARAMETER"
%token PERIODS "PERIODS"
%token VIEW_MATRIX "VIEW_MATRIX"
%token VERTICES "VERTICES"
%token EDGES "EDGES"
%token FACES "FACES"
%token BODIES "BODIES"
%token READ "READ"
 // extra attributes
%token DEFINE "DEFINE"
%token ATTRIBUTE "ATTRIBUTE"
%token EDGE "EDGE"
%token FACET "FACET"
%token BODY "BODY"
%token INTEGER_TYPE "INTEGER"
%token REAL_TYPE "REAL"
 // colors
%token COLOR "COLOR"
%token BLACK "BLACK"
%token BLUE "BLUE"
%token GREEN "GREEN"
%token CYAN "CYAN"
%token RED "RED"
%token MAGENTA "MAGENTA"
%token BROWN "BROWN"
%token LIGHTGRAY "LIGHTGRAY"
%token DARKGRAY "DARKGRAY"
%token LIGHTBLUE "LIGHTBLUE"
%token LIGHTGREEN "LIGHTGREEN"
%token LIGHTCYAN "LIGHTCYAN"
%token LIGHTRED "LIGHTRED"
%token LIGHTMAGENTA "LIGHTMAGENTA"
%token YELLOW "YELLOW"
%token WHITE "WHITE"
%token CLEAR "CLEAR"
 // additional parameters
%token STRING "STRING"
%token SOAPFILM "SOAPFILM"
%token LINEAR "LINEAR"
%token QUADRATIC "QUADRATIC"
%token SIMPLEX_REPRESENTATION "SIMPLEX_REPRESENTATION"
%token FIXED "FIXED"
%token TOTAL_TIME "TOTAL_TIME"
%token CONSTRAINT_TOLERANCE "CONSTRAINT_TOLERANCE"
%token SYMMETRIC_CONTENT "SYMMETRIC_CONTENT"
%token SCALE "SCALE"
%token NOWALL "NOWALL"
%token GLOBAL "GLOBAL"
%token CONTENT "CONTENT"
%token C1 "C1"
%token C2 "C2"
%token C3 "C3"
%token CONVEX "CONVEX"
%token ENERGY "ENERGY"
%token E1 "E1"
%token E2 "E2"
%token E3 "E3"
%token FUNCTION "FUNCTION"
%token NONNEGATIVE "NONNEGATIVE"
%token NONPOSITIVE "NONPOSITIVE"
%token FORMULA "FORMULA"
%token CONSTRAINT "CONSTRAINT"
%token EQUATION "EQUATION"
%token TORUS_FILLED "TORUS_FILLED"
%token TORUS "TORUS"
%token ORIGINAL "ORIGINAL"
%token VOLUME "VOLUME"
%token LAGRANGE_MULTIPLIER "LAGRANGE_MULTIPLIER"

 // terminal symbols
%token INTEGER_VALUE
%token REAL_VALUE
%token IDENTIFIER

 // operator precedence
%right '='
%left '-' '+'
%left '*' '/'
%left NEGATION  /* negation--unary minus */
%right '^'      /* exponentiation */


%%
datafile: header vertices edges faces bodies eof 
{fprintf(stdout, "File parsed successfully\n")};

eof: READ
|
;

header: header parameter
| header attribute              
| header dimensionality         
| header representation         
| header scale_factor           
| header total_time             
| header constraint_tolerance   
| header SYMMETRIC_CONTENT      
| header view_matrix            
| header constraint             
| header torus_domain           
|
;

parameter: PARAMETER IDENTIFIER '=' const_expr;

attribute: DEFINE element_type ATTRIBUTE IDENTIFIER attribute_value_type;

element_type: EDGE | FACET | BODY;

attribute_value_type: INTEGER_TYPE 
| REAL_TYPE 
| INTEGER_TYPE '[' INTEGER_VALUE ']' 
| REAL_TYPE '[' INTEGER_VALUE ']';

dimensionality: STRING | SOAPFILM;

representation: LINEAR | QUADRATIC | SIMPLEX_REPRESENTATION;

scale_factor: SCALE ':' const_expr;

total_time: TOTAL_TIME const_expr;

constraint_tolerance: CONSTRAINT_TOLERANCE ':' REAL_VALUE;

view_matrix: VIEW_MATRIX
const_expr const_expr const_expr const_expr
const_expr const_expr const_expr const_expr
const_expr const_expr const_expr const_expr
const_expr const_expr const_expr const_expr;


constraint: CONSTRAINT INTEGER_VALUE constraint_params 
constraint_type ':' expr 
constraint_energy     
constraint_content    
;

constraint_params: constraint_params GLOBAL
| constraint_params CONVEX
| constraint_params NONNEGATIVE
| constraint_params NONPOSITIVE
| constraint_params NOWALL
|;

constraint_type: EQUATION | FORMULA | FUNCTION;

constraint_energy: ENERGY E1 ':' expr E2 ':' expr E3 ':' expr |;

constraint_content: CONTENT 
C1 ':' expr 
C2 ':' expr 
C3 ':' expr 
|;

const_expr: expr;

expr:     number
        | IDENTIFIER
        | IDENTIFIER '(' expr ')'
        | expr '=' expr
        | expr '+' expr
        | expr '-' expr
        | expr '*' expr
        | expr '/' expr
        | '-' expr  %prec NEGATION
        | expr '^' expr
        | '(' expr ')'
;

number: INTEGER_VALUE | REAL_VALUE;

number_list: number | number_list ',' number

torus_domain: torus_type torus_periods;

torus_type: TORUS | TORUS_FILLED;
torus_periods: PERIODS
const_expr const_expr const_expr
const_expr const_expr const_expr
const_expr const_expr const_expr

vertices: VERTICES vertex_list;

vertex_list: vertex_list
             INTEGER_VALUE number number number vertex_attribute_list
| ;

vertex_attribute_list: vertex_attribute_list predefined_vertex_attribute
| vertex_attribute_list user_attribute
| ;

predefined_vertex_attribute: ORIGINAL INTEGER_VALUE;

user_attribute: IDENTIFIER number | IDENTIFIER '{' number_list '}';

edges: EDGES edge_list;
edge_list: edge_list
           INTEGER_VALUE INTEGER_VALUE INTEGER_VALUE 
           signs_torus_model edge_attribute_list
| ;

edge_attribute_list: edge_attribute_list predefined_edge_attribute
| edge_attribute_list user_attribute
| ;

predefined_edge_attribute: ORIGINAL INTEGER_VALUE;

signs_torus_model: sign_torus_model sign_torus_model sign_torus_model
|;

sign_torus_model: '+' | '*' | '-';

faces: FACES face_list;


face_list: face_list
           INTEGER_VALUE value_list face_attribute_list 
| ;

face_attribute_list: face_attribute_list predefined_face_attribute
| face_attribute_list user_attribute
| ;

predefined_face_attribute: COLOR color_name 
| ORIGINAL INTEGER_VALUE;

color_name: BLACK
| BLUE
| GREEN
| CYAN
| RED
| MAGENTA
| BROWN
| LIGHTGRAY
| DARKGRAY
| LIGHTBLUE
| LIGHTGREEN
| LIGHTCYAN
| LIGHTRED
| LIGHTMAGENTA
| YELLOW
| WHITE
| CLEAR;


bodies: BODIES body_list;

body_list: body_list
INTEGER_VALUE value_list 
body_attribute_list
| ;

value_list: value_list INTEGER_VALUE
| ;

body_attribute_list: body_attribute_list predefined_body_attribute
| body_attribute_list user_attribute
| ;

predefined_body_attribute: VOLUME number
| LAGRANGE_MULTIPLIER number 
| ORIGINAL INTEGER_VALUE;


%%

/**
 * Standard bison function, called when there is a 
 * parse error.
 * @param error error message
 */
void yyerror (char const *error)
{
    fprintf (stderr, "%s at line %d\n", error, yylloc.first_line);
}

int KeywordId (char* keyword)
{
    const int yytnameDisplacement = 255;
    int i;
    for (i = 0; i < YYNTOKENS; i++)
    {
	if (yytname[i] != 0
	    && yytname[i][0] == '"'
	    && ! strncasecmp (yytname[i] + 1, keyword,
			  strlen (keyword))
	    && yytname[i][strlen (keyword) + 1] == '"'
	    && yytname[i][strlen (keyword) + 2] == 0)
	    return yytnameDisplacement + i;
    }
    return 0;
}

/**
 * Turns on or off bison debugginig
 * @param debugging turns on debugging for 1, off for 0
 */
static void bisonDebugging (int debugging)
{
    yydebug = debugging;
}


/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(void)
{
    FlexDebugging (0);
    bisonDebugging (0);
    return yyparse();
}
