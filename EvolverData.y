/**
 * @file   foam.y
 * @author Dan R. Lipsa
 *
 * Grammar  description for  the parser  used to  read in  a  DMP file
 * produced by the Surface Evolver software.
 */
%skeleton "lalr1.cc"                          /*  -*- C++ -*- */
%require "2.1a"
%defines
%token-table 
%error-verbose
%expect 4 // state 55, 64, 69, 201
%locations
%{
class Foam;
class ParsingDriver;
%}
%parse-param { Foam& foam }
%parse-param { void* scanner }
%lex-param   { void* scanner }
%initial-action
{
    // Initialize the initial location.
    @$.begin.filename = @$.end.filename = &foam.GetParsingData ().GetFile ();
}

%{
#include "Enums.h"
class ExpressionTree;
class NameSemanticValue;
class AttributeCreator;
%}

/**
 * Defines the datatypes for semantic values for terminals and non-terminals
 * in the gramar
 */
%union
{
    short m_short;
    /**
     * Value for an iteger
     */
    int m_int;
    /**
     * Value for a floating point
     */
    float m_real;
    /**
     * Value for a list of integers
     */
    std::vector<int>* m_intList;
    /**
     * Value for a list of real numbers
     */
    std::vector<float>* m_realList;
    /**
     * An identifier
     */
    std::string* m_id;
    /**
     * An expression tree
     */
    ExpressionTree* m_node;
    /**
     * Knows how to create an attribute
     */
    AttributeCreator* m_attributeCreator;
    DefineAttribute::Type m_attributeType;
    std::vector<NameSemanticValue*>* m_nameSemanticValueList;
    NameSemanticValue* m_nameSemanticValue;
    G3D::Vector3int16* m_vector3int16;
    Color::Name m_color;
}



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
%token <m_id> COLOR "COLOR"
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
%token KEEP_ORIGINALS "KEEP_ORIGINALS"
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
%token SPACE_DIMENSION "SPACE_DIMENSION"
%token LENGTH_METHOD_NAME "LENGTH_METHOD_NAME"
%token AREA_METHOD_NAME "AREA_METHOD_NAME"
%token QUANTITY "QUANTITY"
%token METHOD_INSTANCE "METHOD_INSTANCE"
%token METHOD "METHOD"
%token FIXED "FIXED"
%token NO_REFINE "NO_REFINE"
%token SCALAR_INTEGRAND "SCALAR_INTEGRAND"
%token VIEW_TRANSFORM_GENERATORS "VIEW_TRANSFORM_GENERATORS"
%token SWAP_COLORS "SWAP_COLORS"
%token INTEGRAL_ORDER_1D "INTEGRAL_ORDER_1D"
%token <m_id> ORIGINAL "ORIGINAL"
%token <m_id> VOLUME "VOLUME"
%token <m_id> VOLCONST "VOLCONST"
%token <m_id> LAGRANGE_MULTIPLIER "LAGRANGE_MULTIPLIER"
%token <m_id> CONSTRAINTS "CONSTRAINTS"
%token <m_id> DENSITY "DENSITY"


 // terminal symbols
%token <m_int> INTEGER_VALUE
%token <m_real> REAL_VALUE
%token <m_id> IDENTIFIER
%token <m_id> ATTRIBUTE_ID
%token <m_id> METHOD_OR_QUANTITY_ID
%token <m_id> '='
%token <m_id> '?'
 /* arithmetic */
%token <m_id> '+'
%token <m_id> '-'
%token <m_id> '*'
%token <m_id> '/'
%token <m_id> '^'
 /*comparison */
%token <m_id> GE ">="
%token <m_id> '>'
%token <m_id> '<'
%token <m_id> LE "<="
 /* logical */
%token <m_id> '!'
%token <m_id> AND "&&"
%token <m_id> OR "||"

 // operator precedence
%right '='
%right '?' ':'
%left OR
%left AND
%left '>' '<' LE GE
%left '-' '+'
%left '*' '/'
%right '^'      /* exponentiation */
%right '!' UMINUS  /* negation--unary minus */

%type <m_real> const_expr
%type <m_real> vertex_list_rest
%type <m_attributeType> element_type
%type <m_attributeCreator> attribute_value_type
%type <m_node> expr
%type <m_real> number
%type <m_nameSemanticValueList> vertex_attribute_list
%type <m_nameSemanticValueList> edge_attribute_list
%type <m_nameSemanticValueList> face_attribute_list
%type <m_nameSemanticValueList> body_attribute_list
%type <m_nameSemanticValue> predefined_vertex_attribute
%type <m_nameSemanticValue> predefined_edge_attribute
%type <m_nameSemanticValue> predefined_face_attribute
%type <m_nameSemanticValue> predefined_body_attribute
%type <m_nameSemanticValue> user_attribute
%type <m_nameSemanticValue> method_or_quantity
%type <m_intList> comma_integer_list
%type <m_intList> integer_list
%type <m_realList> comma_real_list
%type <m_color> color_name
%type <m_vector3int16> signs_torus_model
%type <m_short> sign_torus_model
%type <m_short> opt_sign_torus_model
%type <m_int> quantity_rest
%type <m_int> method_instance_rest

%{
#include "Foam.h"
#include "Debug.h"
#include "Edge.h"
#include "ParsingData.h"
#include "ExpressionTree.h"
#include "AttributeCreator.h"
#include "SystemDifferences.h"
#include "NameSemanticValue.h"

int yylex(void);
void yyerror (char const *);
/**
 * Converts an int to an unsigned int and print a message if the int is negative
 * @param i the integer to be converted
 * @param message the message to printed if the integer is negative
 * @return the unsigned integer.
 */
size_t intToUnsigned (int i, const char* message);

%}

%%
datafile
: nlstar header vertices nlstar
{
    //foam.GetParsingData ().PrintTimeCheckpoint ("After vertices:");
}
edges nlstar
{
    //foam.GetParsingData ().PrintTimeCheckpoint ("After edges:");
}
faces nlstar
{
    //foam.GetParsingData ().PrintTimeCheckpoint ("After faces:");
}
bodies nlstar
{
    //foam.GetParsingData ().PrintTimeCheckpoint ("After bodies:");
    foam.PostProcess ();
}

header
: /* empty */
| header dimensionality nlplus
| header space_dimension  nlplus      
| header parameter nlplus
| header attribute nlplus              
| header representation nlplus
| header scale_factor nlplus   
| header total_time nlplus
| header constraint_tolerance nlplus
| header SYMMETRIC_CONTENT nlplus
| header KEEP_ORIGINALS nlplus
| header view_matrix nlplus   
| header constraint
| header torus_domain nlplus
| header length_method_name nlplus
| header area_method_name nlplus
| header quantity
| header method_instance
| header function_declaration nlplus
| header view_transform_generators
| header integral_order_1d nlplus
;

nl: '\n'
;

nlstar
: /* empty */
| nlstar nl 
;

nlplus
: nlplus nl 
| nl
;

integral_order_1d
: INTEGRAL_ORDER_1D ':' INTEGER_VALUE
;

view_transform_generators
: VIEW_TRANSFORM_GENERATORS INTEGER_VALUE nlplus
  swap_colors
  view_transform_generators_matrices
;

swap_colors
: /* empty */
| SWAP_COLORS nlplus 
;

view_transform_generators_matrices
: view_transform_generators_matrices view_transform_generators_matrix nlplus 
| view_transform_generators_matrix nlplus
;

/* 2D or 3D */
view_transform_generators_matrix
: const_expr const_expr opt_const_expr nlplus
  const_expr const_expr opt_const_expr nlplus
  const_expr const_expr opt_const_expr
| const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr
;

opt_const_expr
: /* empty */
| const_expr 
;

length_method_name
: LENGTH_METHOD_NAME '"' IDENTIFIER '"'
;

area_method_name
: AREA_METHOD_NAME '"' IDENTIFIER '"'
;

quantity
: QUANTITY IDENTIFIER quantity_rest
{
    if ($3 != 0)
	foam.GetParsingData ().AddMethodOrQuantity ($2->c_str ());
}

quantity_rest
: ';' nlplus 
{
    $$ = 0;
}
| FIXED '=' REAL_VALUE LAGRANGE_MULTIPLIER number quantity_method_list nlplus
{
    $$ = 1;
}

quantity_method_list
: quantity_method_list quantity_method 
| quantity_method
;

quantity_method
: METHOD METHOD_OR_QUANTITY_ID
;

method_instance: 
METHOD_INSTANCE IDENTIFIER method_instance_rest
{
    if ($3 != 0)
	foam.GetParsingData ().AddMethodOrQuantity ($2->c_str ());
}

method_instance_rest
: ';' nlplus
{
    $$ = 0;
}
| METHOD IDENTIFIER nlplus method_instance_parameters
{
    $$ = 1;
}

method_instance_parameters
: /* empty */
| SCALAR_INTEGRAND ':' expr nlplus 
;

function_declaration
: FUNCTION function_return_type IDENTIFIER '(' 
  function_parameters ')' ';'
;

function_return_type
: INTEGER_TYPE 
| REAL_TYPE
;

function_parameters
: function_parameters ',' function_parameter 
| function_parameter
;

function_parameter
: function_parameter_type IDENTIFIER
;

function_parameter_type
: INTEGER_TYPE 
| REAL_TYPE
;

parameter: PARAMETER IDENTIFIER '=' const_expr
{
    float v = $4;
    foam.GetParsingData ().SetVariable($2->c_str(), v);
}


attribute
: DEFINE element_type ATTRIBUTE IDENTIFIER attribute_value_type
{
    auto_ptr<AttributeCreator> ac ($5);
    foam.AddAttributeInfo ($2, $4->c_str(), ac);
}

element_type: VERTEX
{
    $$ = DefineAttribute::VERTEX;
}
| EDGE 
{
    $$ = DefineAttribute::EDGE;
}
| FACET 
{
    $$ = DefineAttribute::FACE;
}
| BODY
{
    $$ = DefineAttribute::BODY;
}

attribute_value_type
: INTEGER_TYPE
{
    $$ = new IntegerAttributeCreator ();
}
| REAL_TYPE 
{
    $$ = new RealAttributeCreator ();
}
| INTEGER_TYPE '[' INTEGER_VALUE ']'
{
    $$ = new IntegerArrayAttributeCreator ($3);
}
| REAL_TYPE '[' INTEGER_VALUE ']'
{
    $$ = new RealArrayAttributeCreator ($3);
}

dimensionality
: STRING 
| SOAPFILM
;

representation
: LINEAR 
| QUADRATIC 
| SIMPLEX_REPRESENTATION
;

scale_factor
: SCALE ':' const_expr
;

total_time
: TOTAL_TIME const_expr
;

constraint_tolerance
: CONSTRAINT_TOLERANCE ':' REAL_VALUE
;

space_dimension
: SPACE_DIMENSION const_expr
{
    foam.SetSpaceDimension ($2);
}

/* 2D or 3D */
view_matrix
: VIEW_MATRIX nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr
{
    foam.SetViewMatrixElement (0, $3);
    foam.SetViewMatrixElement (1, $4);
    foam.SetViewMatrixElement (2, $5);
    foam.SetViewMatrixElement (3, $6);

    foam.SetViewMatrixElement (4, $8);
    foam.SetViewMatrixElement (5, $9);
    foam.SetViewMatrixElement (6, $10);
    foam.SetViewMatrixElement (7, $11);

    foam.SetViewMatrixElement (8, $13);
    foam.SetViewMatrixElement (9, $14);
    foam.SetViewMatrixElement (10, $15);
    foam.SetViewMatrixElement (11, $16);

    foam.SetViewMatrixElement (12, $18);
    foam.SetViewMatrixElement (13, $19);
    foam.SetViewMatrixElement (14, $20);
    foam.SetViewMatrixElement (15, $21);
}
| VIEW_MATRIX nlplus
  const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr
{
    foam.SetViewMatrixElement (0, $3);
    foam.SetViewMatrixElement (1, $4);
    foam.SetViewMatrixElement (2, $5);

    foam.SetViewMatrixElement (4, $7);
    foam.SetViewMatrixElement (5, $8);
    foam.SetViewMatrixElement (6, $9);

    foam.SetViewMatrixElement (8, $11);
    foam.SetViewMatrixElement (9, $12);
    foam.SetViewMatrixElement (10, $13);
}

constraint
: CONSTRAINT INTEGER_VALUE constraint_params nlplus
  constraint_type ':' non_const_expr nlplus
  constraint_energy
  constraint_content
;

constraint_params
: /* empty */
| constraint_params GLOBAL
| constraint_params CONVEX
| constraint_params NONNEGATIVE
| constraint_params NONPOSITIVE
| constraint_params NOWALL
;

constraint_type
: EQUATION 
| FORMULA 
| FUNCTION
;

/* 2D or 3D */
constraint_energy
: /* empty */
| ENERGY nl
  E1 ':' non_const_expr nlplus constraint_energy_rest
;

constraint_energy_rest
: /* empty */
| E2 ':' non_const_expr nlplus
  E3 ':' non_const_expr nlplus
;

/* 2D or 3D */
constraint_content
: /* empty */
| CONTENT nl
  C1 ':' non_const_expr nlplus constraint_content_rest
;

constraint_content_rest
: /* empty */
| C2 ':' non_const_expr nlplus
  C3 ':' non_const_expr nlplus
;

non_const_expr
: expr
{
    ExpressionTree::Delete ($1);
}

const_expr
: expr
{
    float v = $1->Value ();
    $$ = v;
    ExpressionTree::Delete ($1);
}

expr
: number
{
    $$ = new ExpressionTreeNumber ($1);
}
| IDENTIFIER
{
    $$ = new ExpressionTreeVariable ($1, foam.GetParsingData ());
}

/* Function calls */
| IDENTIFIER '(' expr ')'
{
    $$ = new ExpressionTreeUnaryFunction ($1, $3, foam.GetParsingData ());
}
| IDENTIFIER '(' expr ',' expr ')'
{
    $$ = new ExpressionTreeBinaryFunction ($1, $3, $5, foam.GetParsingData ());
}

/* Arithmetic operations */
| '-' expr  %prec UMINUS
{
    $$ = new ExpressionTreeUnaryFunction ($1, $2, foam.GetParsingData ());
}
| expr '+' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '-' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '*' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '/' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '^' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}

/* Comparisions */
| expr '>' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr GE expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '<' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr LE expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}

/* Logical operations */
| '!' expr
{
    $$ = new ExpressionTreeUnaryFunction ($1, $2, foam.GetParsingData ());
}
| expr AND expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr OR expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}

/* Other expressions */
| '(' expr ')'
{
    $$ = $2;
}
| expr '=' expr
{
    $$ = new ExpressionTreeBinaryFunction ($2, $1, $3, foam.GetParsingData ());
}
| expr '?' expr ':' expr
{
    $$ = new ExpressionTreeConditional ($1, $3, $5, foam.GetParsingData ());
}

number
: INTEGER_VALUE 
{
    $$ = $1;
}
| REAL_VALUE
{
    $$ = $1;
}

torus_domain
: torus_type nlplus torus_periods
;

torus_type
: TORUS 
| TORUS_FILLED
;

/* 2D or 3D */
torus_periods
: PERIODS nl
  const_expr const_expr nl
  const_expr const_expr
{
    using G3D::Vector3;
    foam.SetPeriods (Vector3 ($3, $4, 0),
		     Vector3 ($6, $7, 0),
		     Vector3::unitZ ());
}
| PERIODS nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr
{
    using G3D::Vector3;
    foam.SetPeriods (Vector3 ($3, $4, $5),
		     Vector3 ($7, $8, $9),
		     Vector3 ($11, $12, $13));
}

vertices
: VERTICES nlplus vertex_list
;

/* 2D or 3D */
vertex_list
: /* empty */
| vertex_list INTEGER_VALUE const_expr const_expr vertex_list_rest 
  vertex_attribute_list nl
{
    vector<NameSemanticValue*>* nameSemanticValueList = 
	$6;
    foam.SetVertex (
	intToUnsigned($2- 1,
		      "Semantic error: vertex index less than 0: "),
	$3, $4, $5, *nameSemanticValueList);
    if (nameSemanticValueList != 0)
	NameSemanticValue::DeleteVector(nameSemanticValueList);
}


vertex_list_rest
: /* empty */
{
    $$ = 0;
}
|
const_expr
{
    $$ = $1;    
}

vertex_attribute_list
: /* empty */
{
    $$ = 0;
}
| vertex_attribute_list predefined_vertex_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| vertex_attribute_list user_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| vertex_attribute_list method_or_quantity
{
    // ignore the method or quantity name
    $$ = $1;
}

method_or_quantity
: METHOD_OR_QUANTITY_ID method_or_quantity_sign
{
    $$ = 0;
}

method_or_quantity_sign 
: /* empty */
| '-' 
;


predefined_vertex_attribute
: ORIGINAL INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| FIXED
{
    $$ = 0;
}
| CONSTRAINTS integer_list
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}

user_attribute
: ATTRIBUTE_ID INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1->c_str(), $2);
}
| ATTRIBUTE_ID REAL_VALUE
{
    $$ = 
	new NameSemanticValue ($1->c_str(), $2);
}
| ATTRIBUTE_ID '{' comma_integer_list '}'
{
    $$ = 
	new NameSemanticValue ($1->c_str(), $3);
}
| ATTRIBUTE_ID '{' comma_real_list '}'
{
    $$ = 
	new NameSemanticValue ($1->c_str(), $3);
}

edges
: EDGES nlplus edge_list
;

edge_list
: /* empty */
| edge_list INTEGER_VALUE INTEGER_VALUE INTEGER_VALUE edge_midpoint 
  signs_torus_model edge_attribute_list nl
{
    foam.SetEdge (
	intToUnsigned($2 - 1, "Semantic error: edge index less than 0: "),
	intToUnsigned($3 - 1, "Semantic error: edge begin less than 0: "),
	intToUnsigned($4 - 1, "Semantic error: edge end less than 0: "),
	*$6,
	*$7);
    delete $6;
    NameSemanticValue::DeleteVector($7);
}

edge_midpoint
: /* empty */
| INTEGER_VALUE

edge_attribute_list
: /* empty */
{
    $$ = 0;
}
| edge_attribute_list predefined_edge_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| edge_attribute_list user_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| edge_attribute_list method_or_quantity
{
    // ignore the method or quantity name
    $$ = $1;
}

predefined_edge_attribute
: ORIGINAL INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| COLOR color_name 
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| CONSTRAINTS integer_list
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| DENSITY const_expr
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| FIXED
{
    $$ = 0;
}
| NO_REFINE
{
    $$ = 0;
}

signs_torus_model
: sign_torus_model sign_torus_model opt_sign_torus_model
{
    $$ = new G3D::Vector3int16 ($1, $2, $3);
}
|
{
    $$ = new G3D::Vector3int16 (0,0,0);
}

opt_sign_torus_model
: sign_torus_model
{
    $$ = $1;
}
|
{
    $$ = Edge::LocationCharToNumber ('*');
}


sign_torus_model
: '+' 
{
    $$ = Edge::LocationCharToNumber((*$1)[0]);
}
| '*' 
{
    $$ = Edge::LocationCharToNumber((*$1)[0]);
}
| '-'
{
    $$ = Edge::LocationCharToNumber((*$1)[0]);
}
;

faces
: FACES nl face_list
;

face_list
: /* empty */
| face_list INTEGER_VALUE integer_list face_attribute_list nl
{
    vector<int>* intList = $3;
    foam.SetFace (
	intToUnsigned($2- 1, "Semantic error: face index less than 0"), 
	*intList, *$4);
    delete intList;
    NameSemanticValue::DeleteVector($4);
}

face_attribute_list
: /* empty */
{
    $$ = 0;
}
| face_attribute_list predefined_face_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| face_attribute_list user_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}

predefined_face_attribute
: COLOR color_name 
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| ORIGINAL INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}

color_name
: BLACK
{
    $$ = Color::BLACK;
}
| BLUE
{
    $$ = Color::BLUE;
}
| GREEN
{
    $$ = Color::GREEN;
}
| CYAN
{
    $$ = Color::CYAN;
}
| RED
{
    $$ = Color::RED;
}
| MAGENTA
{
    $$ = Color::MAGENTA;
}
| YELLOW
{
    $$ = Color::YELLOW;
}
| WHITE
{
    $$ = Color::WHITE;
}
| CLEAR
{
    $$ = Color::CLEAR;
}
| BROWN
{
    $$ = Color::BROWN;
}
| LIGHTGRAY
{
    $$ = Color::LIGHTGRAY;
}
| DARKGRAY
{
    $$ = Color::DARKGRAY;
}
| LIGHTBLUE
{
    $$ = Color::LIGHTBLUE;
}
| LIGHTGREEN
{
    $$ = Color::LIGHTGREEN;
}
| LIGHTCYAN
{
    $$ = Color::LIGHTCYAN;
}
| LIGHTRED
{
    $$ = Color::LIGHTRED;
}
| LIGHTMAGENTA
{
    $$ = Color::LIGHTMAGENTA;
}


bodies
: BODIES nl body_list
;

body_list
: /* empty */
| body_list INTEGER_VALUE integer_list body_attribute_list nl
{
    vector<int>* intList = $3;
    foam.SetBody (
	intToUnsigned($2- 1, "Semantic error: body index less than 0"),
	*intList, *$4);
    delete intList;
    NameSemanticValue::DeleteVector ($4);
}

integer_list: integer_list INTEGER_VALUE
{
    vector<int>* intList = $1;
    intList->push_back ($2);
    $$ = intList;
}
| INTEGER_VALUE
{
    $$ = new vector<int>(1, $1);
}

comma_integer_list
: comma_integer_list ',' INTEGER_VALUE
{
    vector<int>* intList = $1;
    intList->push_back ($3);
    $$ = intList;
}
| INTEGER_VALUE
{
    $$ = new vector<int>(1, $1);
}

comma_real_list
: comma_real_list ',' REAL_VALUE
{
    vector<float>* realList = $1;
    realList->push_back ($3);
    $$ = realList;
}
| REAL_VALUE
{
    $$ = new vector<float>(1, $1);
}


body_attribute_list
: /* empty */ 
{
    $$ = 0;
}
| body_attribute_list predefined_body_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}
| body_attribute_list user_attribute
{
    $$ = NameSemanticValue::PushBack ($1, $2);
}

predefined_body_attribute
: VOLUME number
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| LAGRANGE_MULTIPLIER number 
{
    $$ = new NameSemanticValue ($1->c_str (), $2);
}
| ORIGINAL INTEGER_VALUE
{
    $$ = 
	new NameSemanticValue ($1->c_str (), $2);
}
| VOLCONST number
{
    $$ = 
	new NameSemanticValue ($1->c_str (), $2);
}


%%

void
EvolverData::parser::error (const EvolverData::parser::location_type& l,
                            const std::string& m)
{
    foam.GetParsingData ().PrintError (l, m);
}


size_t intToUnsigned (int i, const char* message)
{
    RuntimeAssert (i >= 0, message, i);
    return static_cast<size_t>(i);
}

// Local Variables:
// mode: c++
// End:
