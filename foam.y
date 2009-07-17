/**
 * @file   foam.y
 * @author Dan R. Lipsa
 *
 * Grammar  description for  the parser  used to  read in  a  DMP file
 * produced by the Surface Evolver software.
 */
%{
#include <string.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

#include "lexYacc.h"
#include "Data.h"
#include "ParsingData.h"
#include "SemanticError.h"
#include "ExpressionTree.h"

int yylex(void);
void yyerror (char const *);
/**
 * Converts an int to an unsigned int and print a message if the int is negative
 * @param i the integer to be converted
 * @param message the message to printed if the integer is negative
 * @return the unsigned integer.
 */
unsigned int intToUnsigned (int i, const char* message);
using namespace std;

/**
 * STL unary function that converts from 1 based indexing to 0 based indexing.
 */
struct decrementElementIndex : public unary_function<int, int>
{
    /**
     * Converts an index from 1-based to 0-based.
     * Decrements the index if it is positive and increments it if it
     * is negative.
     * @param i index to be converted
     * @return the converted index
     */
    int operator() (int i)
    {
	if (i < 0)
	    i++;
	else if (i > 0)
	    i--;
	else
	    throw SemanticError ("Semantic error: invalid element index: 0");
	return i;
    }
};

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
datafile: header vertices edges faces bodies eof;

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

parameter: PARAMETER IDENTIFIER '=' const_expr
{
    float v = $4.r;
    data.GetParsingData ().SetVariable($2.id->c_str(), v);
}
;

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
const_expr const_expr const_expr const_expr
{
    data.SetViewMatrixElement (0, $2.r);
    data.SetViewMatrixElement (1, $3.r);
    data.SetViewMatrixElement (2, $4.r);
    data.SetViewMatrixElement (3, $5.r);
    data.SetViewMatrixElement (4, $6.r);
    data.SetViewMatrixElement (5, $7.r);
    data.SetViewMatrixElement (6, $8.r);
    data.SetViewMatrixElement (7, $9.r);
    data.SetViewMatrixElement (8, $10.r);
    data.SetViewMatrixElement (9, $11.r);
    data.SetViewMatrixElement (10, $12.r);
    data.SetViewMatrixElement (11, $13.r);
    data.SetViewMatrixElement (12, $14.r);
    data.SetViewMatrixElement (13, $15.r);
    data.SetViewMatrixElement (14, $16.r);
    data.SetViewMatrixElement (15, $17.r);
};


constraint: CONSTRAINT INTEGER_VALUE constraint_params 
constraint_type ':' non_const_expr 
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

constraint_energy: ENERGY 
E1 ':' non_const_expr 
E2 ':' non_const_expr 
E3 ':' non_const_expr 
|;

constraint_content: CONTENT 
C1 ':' non_const_expr 
C2 ':' non_const_expr 
C3 ':' non_const_expr 
|;

non_const_expr: expr
{
    ExpressionTree::Delete ($1.node);
}
;

const_expr: expr
{
    float v = $1.node->Value ();
    $$.r = v;
    ExpressionTree::Delete ($1.node);
};

expr:     number
{
    $$.node = new ExpressionTreeNumber ($1.r);
}
| IDENTIFIER
{
    $$.node = new ExpressionTreeVariable ($1.id, data.GetParsingData ());
}
| IDENTIFIER '(' expr ')'
{
    $$.node = new ExpressionTreeUnaryFunction (
	$1.id, $3.node, data.GetParsingData ());
}
| expr '+' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
}
| expr '-' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
}
| expr '*' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
}
| expr '/' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
}
| '-' expr  %prec NEGATION
{
    $$.node = new ExpressionTreeUnaryFunction (
	$1.id, $2.node, data.GetParsingData ());
}
| expr '^' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
}
| '(' expr ')'
{
    $$.node = $2.node;
}
| expr '=' expr
{
    $$.node = new ExpressionTreeBinaryFunction (
	$2.id, $1.node, $3.node, data.GetParsingData ());
};

number: INTEGER_VALUE 
{
    $$.r = $1.i;
}
| REAL_VALUE
{
    $$.r = $1.r;
};

number_list: number | number_list ',' number

torus_domain: torus_type torus_periods;

torus_type: TORUS | TORUS_FILLED;
torus_periods: PERIODS
const_expr const_expr const_expr
const_expr const_expr const_expr
const_expr const_expr const_expr

vertices: VERTICES vertex_list;

vertex_list: vertex_list INTEGER_VALUE number number number 
vertex_attribute_list
{
    data.SetPoint (
	intToUnsigned($2.i - 1,
		      "Semantic error: vertex index less than 0: "),
	$3.r, $4.r, $5.r);
}
|;

vertex_attribute_list: vertex_attribute_list predefined_vertex_attribute
| vertex_attribute_list user_attribute
| ;

predefined_vertex_attribute: ORIGINAL INTEGER_VALUE;

user_attribute: IDENTIFIER number | IDENTIFIER '{' number_list '}';

edges: EDGES edge_list;
edge_list: edge_list INTEGER_VALUE INTEGER_VALUE INTEGER_VALUE signs_torus_model edge_attribute_list
{
    data.SetEdge (
	intToUnsigned($2.i - 1, "Semantic error: edge index less than 0: "),
	intToUnsigned($3.i - 1, "Semantic error: edge begin less than 0: "),
	intToUnsigned($4.i - 1, "Semantic error: edge end less than 0: "));
}
| ;

edge_attribute_list: edge_attribute_list predefined_edge_attribute
| edge_attribute_list user_attribute
| ;

predefined_edge_attribute: ORIGINAL INTEGER_VALUE;

signs_torus_model: sign_torus_model sign_torus_model sign_torus_model
|;

sign_torus_model: '+' | '*' | '-';

faces: FACES face_list;

face_list: face_list INTEGER_VALUE integer_list face_attribute_list
{
    vector<int>* intList = $3.intList;
    transform(intList->begin(), intList->end(), intList->begin(),
	      decrementElementIndex());
    data.SetFace (
	intToUnsigned($2.i - 1, "Semantic error: face index less than 0"), 
	*intList);
    delete intList;
}
|;

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

body_list: body_list INTEGER_VALUE integer_list body_attribute_list
{
    vector<int>* intList = $3.intList;
    transform(intList->begin (), intList->end (), intList->begin (),
	      decrementElementIndex ());
    data.SetBody (
	intToUnsigned($2.i - 1, "Semantic error: body index less than 0"),
	*intList);
    delete intList;
}
| ;

integer_list: integer_list INTEGER_VALUE
{
    vector<int>* intList = $1.intList;
    if (intList == 0)
	intList = new vector<int>();
    intList->push_back ($2.i);
    $$.intList = intList;
}
| {$$.intList = 0}
;

body_attribute_list: body_attribute_list predefined_body_attribute
| body_attribute_list user_attribute
| ;

predefined_body_attribute: VOLUME number
| LAGRANGE_MULTIPLIER number 
| ORIGINAL INTEGER_VALUE;


%%

void yyerror (char const *error)
{
    cerr << error << " at line " << yylloc.first_line << endl;
}

unsigned int intToUnsigned (int i, const char* message)
{
    if (i < 0)
    {
	ostringstream message;
	message << message << i << ends;
	throw SemanticError (message.str());
    }
    return static_cast<unsigned int>(i);
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

void BisonDebugging (int debugging)
{
    yydebug = debugging;
}

// Local Variables:
// mode: c++
// End:
