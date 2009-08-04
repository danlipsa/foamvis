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
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <Qt>
#include "lexYacc.h"
#include "Data.h"
#include "ParsingData.h"
#include "SemanticError.h"
#include "ExpressionTree.h"
#include "AttributeCreator.h"
using namespace std;

int yylex(void);
void yyerror (char const *);
/**
 * Converts an int to an unsigned int and print a message if the int is negative
 * @param i the integer to be converted
 * @param message the message to printed if the integer is negative
 * @return the unsigned integer.
 */
unsigned int intToUnsigned (int i, const char* message);
/**
 * Converts an index from 1-based to 0-based.
 * Decrements the index if it is positive and increments it if it
 * is negative.
 * @param i index to be converted
 * @return the converted index
 */
int decrementElementIndex (int i);

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
%token VERTEX "VERTEX"
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
datafile: header vertices edges faces bodies
{
    cerr << "The end" << endl;
    data.Compact ();
};

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
    float v = $4.m_real;
    data.GetParsingData ().SetVariable($2.m_id->c_str(), v);
}
;

attribute: DEFINE element_type ATTRIBUTE IDENTIFIER attribute_value_type
{
    data.AddAttributeInfo (
	$2.m_attributeType, $4.m_id->c_str(), $5.m_attributeCreator);
}
;

element_type: VERTEX
{
    $$.m_attributeType = VERTEX_TYPE;
}
| EDGE 
{
    $$.m_attributeType = EDGE_TYPE;
}
| FACET 
{
    $$.m_attributeType = FACE_TYPE;
}
| BODY
{
    $$.m_attributeType = BODY_TYPE;
}
;

attribute_value_type: INTEGER_TYPE
{
    $$.m_attributeCreator = new IntegerAttributeCreator ();
}
| REAL_TYPE 
{
    $$.m_attributeCreator = new RealAttributeCreator ();
}
| INTEGER_TYPE '[' INTEGER_VALUE ']'
{
    $$.m_attributeCreator = new IntegerArrayAttributeCreator ($3.m_int);
}
| REAL_TYPE '[' INTEGER_VALUE ']'
{
    $$.m_attributeCreator = new RealArrayAttributeCreator ($3.m_int);
}
;


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
    data.SetViewMatrixElement (0, $2.m_real);
    data.SetViewMatrixElement (1, $3.m_real);
    data.SetViewMatrixElement (2, $4.m_real);
    data.SetViewMatrixElement (3, $5.m_real);
    data.SetViewMatrixElement (4, $6.m_real);
    data.SetViewMatrixElement (5, $7.m_real);
    data.SetViewMatrixElement (6, $8.m_real);
    data.SetViewMatrixElement (7, $9.m_real);
    data.SetViewMatrixElement (8, $10.m_real);
    data.SetViewMatrixElement (9, $11.m_real);
    data.SetViewMatrixElement (10, $12.m_real);
    data.SetViewMatrixElement (11, $13.m_real);
    data.SetViewMatrixElement (12, $14.m_real);
    data.SetViewMatrixElement (13, $15.m_real);
    data.SetViewMatrixElement (14, $16.m_real);
    data.SetViewMatrixElement (15, $17.m_real);
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
    ExpressionTree::Delete ($1.m_node);
}
;

const_expr: expr
{
    float v = $1.m_node->Value ();
    $$.m_real = v;
    ExpressionTree::Delete ($1.m_node);
};

expr:     number
{
    $$.m_node = new ExpressionTreeNumber ($1.m_real);
}
| IDENTIFIER
{
    $$.m_node = new ExpressionTreeVariable ($1.m_id, data.GetParsingData ());
}
| IDENTIFIER '(' expr ')'
{
    $$.m_node = new ExpressionTreeUnaryFunction (
	$1.m_id, $3.m_node, data.GetParsingData ());
}
| expr '+' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
}
| expr '-' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
}
| expr '*' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
}
| expr '/' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
}
| '-' expr  %prec NEGATION
{
    $$.m_node = new ExpressionTreeUnaryFunction (
	$1.m_id, $2.m_node, data.GetParsingData ());
}
| expr '^' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
}
| '(' expr ')'
{
    $$.m_node = $2.m_node;
}
| expr '=' expr
{
    $$.m_node = new ExpressionTreeBinaryFunction (
	$2.m_id, $1.m_node, $3.m_node, data.GetParsingData ());
};

number: INTEGER_VALUE 
{
    $$.m_real = $1.m_int;
}
| REAL_VALUE
{
    $$.m_real = $1.m_real;
};

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
    vector<NameSemanticValue*>* nameSemanticValueList = 
	$6.m_nameSemanticValueList;
    data.SetPoint (
	intToUnsigned($2.m_int- 1,
		      "Semantic error: vertex index less than 0: "),
	$3.m_real, $4.m_real, $5.m_real, *nameSemanticValueList);
    if (nameSemanticValueList != 0)
	NameSemanticValue::DeleteVector(nameSemanticValueList);
}
|;

vertex_attribute_list: vertex_attribute_list predefined_vertex_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
| vertex_attribute_list user_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
|
{
    $$.m_nameSemanticValueList = 0;
}
;

predefined_vertex_attribute: ORIGINAL INTEGER_VALUE
{
    $$.m_nameSemanticValue = new NameSemanticValue (
	$1.m_id->c_str (), $2.m_int);
}
;

user_attribute: IDENTIFIER INTEGER_VALUE
{
    $$.m_nameSemanticValue = new NameSemanticValue ($1.m_id->c_str(), $2.m_int);
}
| IDENTIFIER REAL_VALUE
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str(), $2.m_real);
}
| IDENTIFIER '{' comma_integer_list '}'
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str(), $3.m_intList);
}
| IDENTIFIER '{' comma_real_list '}'
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str(), $3.m_realList);
}
;

edges: EDGES edge_list;
edge_list: edge_list INTEGER_VALUE INTEGER_VALUE INTEGER_VALUE 
signs_torus_model edge_attribute_list
{
    data.SetEdge (
	intToUnsigned($2.m_int- 1, "Semantic error: edge index less than 0: "),
	intToUnsigned($3.m_int- 1, "Semantic error: edge begin less than 0: "),
	intToUnsigned($4.m_int- 1, "Semantic error: edge end less than 0: "),
	*$6.m_nameSemanticValueList);
    NameSemanticValue::DeleteVector($6.m_nameSemanticValueList);
}
| ;

edge_attribute_list: edge_attribute_list predefined_edge_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
| edge_attribute_list user_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
|
{
    $$.m_nameSemanticValueList = 0;
}
;

predefined_edge_attribute: ORIGINAL INTEGER_VALUE
{
    $$.m_nameSemanticValue = new NameSemanticValue (
	$1.m_id->c_str (), $2.m_int);
}
;

signs_torus_model: sign_torus_model sign_torus_model sign_torus_model
|;

sign_torus_model: '+' | '*' | '-';

faces: FACES face_list;

face_list: face_list INTEGER_VALUE integer_list face_attribute_list
{
    vector<int>* intList = $3.m_intList;
    transform(intList->begin(), intList->end(), intList->begin(),
	      decrementElementIndex);
    data.SetFace (
	intToUnsigned($2.m_int- 1, "Semantic error: face index less than 0"), 
	*intList, *$4.m_nameSemanticValueList);
    delete intList;
    NameSemanticValue::DeleteVector($4.m_nameSemanticValueList);
}
|;

face_attribute_list: face_attribute_list predefined_face_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
| face_attribute_list user_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
|
{
    $$.m_nameSemanticValueList = 0;
}
;

predefined_face_attribute: COLOR color_name 
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str (), $2.m_color);
}
| ORIGINAL INTEGER_VALUE
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str (), $2.m_int);
};

color_name: BLACK
{
    $$.m_color = Qt::black;
}
| BLUE
{
    $$.m_color = Qt::blue;
}
| GREEN
{
    $$.m_color = Qt::green;
}
| CYAN
{
    $$.m_color = Qt::cyan;
}
| RED
{
    $$.m_color = Qt::red;
}
| MAGENTA
{
    $$.m_color = Qt::magenta;
}
| YELLOW
{
    $$.m_color = Qt::yellow;
}
| WHITE
{
    $$.m_color = Qt::white;
}
| CLEAR
{
    $$.m_color = Qt::transparent;
}
| BROWN
{
    $$.m_color = Qt::darkYellow;
}
| LIGHTGRAY
{
    $$.m_color = Qt::lightGray;
}
| DARKGRAY
{
    $$.m_color = Qt::darkGray;
}
| LIGHTBLUE
{
    $$.m_color = Qt::darkBlue;
}
| LIGHTGREEN
{
    $$.m_color = Qt::darkGreen;
}
| LIGHTCYAN
{
    $$.m_color = Qt::darkCyan;
}
| LIGHTRED
{
    $$.m_color = Qt::darkRed;
}
| LIGHTMAGENTA
{
    $$.m_color = Qt::darkMagenta;
}

;


bodies: BODIES body_list;

body_list: body_list INTEGER_VALUE integer_list body_attribute_list
{
    vector<int>* intList = $3.m_intList;
    transform(intList->begin (), intList->end (), intList->begin (),
	      decrementElementIndex);
    data.SetBody (
	intToUnsigned($2.m_int- 1, "Semantic error: body index less than 0"),
	*intList, *$4.m_nameSemanticValueList);
    delete intList;
    NameSemanticValue::DeleteVector ($4.m_nameSemanticValueList);
}
|;

integer_list: integer_list INTEGER_VALUE
{
    vector<int>* intList = $1.m_intList;
    intList->push_back ($2.m_int);
    $$.m_intList = intList;
}
| INTEGER_VALUE
{
    $$.m_intList = new vector<int>(1, $1.m_int);
}
;

comma_integer_list: comma_integer_list ',' INTEGER_VALUE
{
    vector<int>* intList = $1.m_intList;
    intList->push_back ($3.m_int);
    $$.m_intList = intList;
}
| INTEGER_VALUE
{
    $$.m_intList = new vector<int>(1, $1.m_int);
}
;

comma_real_list: comma_real_list ',' REAL_VALUE
{
    vector<float>* realList = $1.m_realList;
    realList->push_back ($3.m_int);
    $$.m_realList = realList;
}
| REAL_VALUE
{
    $$.m_realList = new vector<float>(1, $1.m_real);
}
;


body_attribute_list: body_attribute_list predefined_body_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
| body_attribute_list user_attribute
{
    $$.m_nameSemanticValueList = 
	$2.m_nameSemanticValue->PushBack ($1.m_nameSemanticValueList);
}
| 
{
    $$.m_nameSemanticValueList = 0;
}
;

predefined_body_attribute: VOLUME number
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str (), $2.m_real);
}
| LAGRANGE_MULTIPLIER number 
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str (), $2.m_real);
}
| ORIGINAL INTEGER_VALUE
{
    $$.m_nameSemanticValue = 
	new NameSemanticValue ($1.m_id->c_str (), $2.m_int);
}
;

%%

void yyerror (char const *error)
{
    cerr << error << " at line " << yylloc.first_line << endl;
}

unsigned int intToUnsigned (int i, const char* message)
{
    if (i < 0)
    {
	ostringstream ostr;
	ostr << message << i << ends;
	throw SemanticError (ostr.str ());
    }
    return static_cast<unsigned int>(i);
}

const int YYTNAME_DISPLACEMENT = 255;

int KeywordId (const char* keyword)
{
    int i;
    for (i = 0; i < YYNTOKENS; i++)
    {
	if (yytname[i] != 0
	    && yytname[i][0] == '"'
	    && ! strncasecmp (yytname[i] + 1, keyword,
			  strlen (keyword))
	    && yytname[i][strlen (keyword) + 1] == '"'
	    && yytname[i][strlen (keyword) + 2] == 0)
	    return YYTNAME_DISPLACEMENT + i;
    }
    return 0;
}

const char* KeywordString (int id)
{
    string str = yytname[id - YYTNAME_DISPLACEMENT];
    str.erase (0, 1);
    str.erase (str.size () - 1, 1);
    return data.GetParsingData ().CreateIdentifier (str.c_str ())->c_str ();
}


void BisonDebugging (int debugging)
{
    yydebug = debugging;
}

int decrementElementIndex (int i)
{
    if (i < 0)
	i++;
    else if (i > 0)
	i--;
    else
	throw SemanticError ("Semantic error: invalid element index: 0");
    return i;
}



// Local Variables:
// mode: c++
// End:
