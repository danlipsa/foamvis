/**
 * @file   EvolverData.y
 * @author Dan R. Lipsa
 * @brief Grammar for parsing a DMP (or FE) file produced by Surface Evolver.
 * @ingroup parser
 *
 * @defgroup parser Parser
 * Files and classes used to parse Surface Evolver .dmp files and 
 * build a memory representation of the simulation file.
 */
/** @cond */
%skeleton "lalr1.cc"                          /*  -*- C++ -*- */
%require "2.1a"
%defines
%token-table
%error-verbose
%expect 3 // state 84, 89, 297
%locations
%{
class Foam;
class ParsingDriver;
%}
%parse-param { Foam* foam }
%parse-param { void* scanner }
%lex-param   { void* scanner }
%initial-action
{
    // Initialize the initial location.
    @$.begin.filename = @$.end.filename = &foam->GetParsingData ().GetFile ();
}

%{
#include "ParsingEnums.h"
class ExpressionTree;
class NameSemanticValue;
class AttributeCreator;
class Attribute;
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
    size_t m_sizet;
    /**
     * Value for a doubleing point
     */
    double m_real;
    /**
     * Value for a list of integers
     */
    std::vector<int>* m_intList;
    std::vector<size_t>* m_sizetList;
    /**
     * Value for a list of real numbers
     */
    std::vector<double>* m_realList;
    /**
     * An identifier
     */
    const char* m_id;
    /**
     * An expression tree
     */
    ExpressionTree* m_node;
    /**
     * Knows how to create an attribute
     */
    AttributeCreator* m_attributeCreator;
    Attribute* m_attribute;
    DefineAttribute::Enum m_attributeType;
    std::vector<NameSemanticValue*>* m_nameSemanticValueList;
    NameSemanticValue* m_nameSemanticValue;
    G3D::Vector3int16* m_vector3int16;
    Color::Enum m_color;
}



// KEYWORDS  -- should come before other tokens --
//    - add the keyword in the table in ParsingDriver.cpp
//    - add the keyword  in the list of  tokens in EvolverData.y
//    - add the correct rule in EvolverData.y
//    - IF NEEDED add the attribute in AddDefault...Attribute in Foam.cpp
%token PARAMETER "PARAMETER"
%token PERIODS "PERIODS"
%token DISPLAY_PERIODS "DISPLAY_PERIODS"
%token DISPLAY_ORIGIN "DISPLAY_ORIGIN"
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
%token TEMPERATURE "TEMPERATURE"
%token CONSTRAINT_TOLERANCE "CONSTRAINT_TOLERANCE"
%token SYMMETRIC_CONTENT "SYMMETRIC_CONTENT"
%token SCALE "SCALE"
%token SCALE_LIMIT "SCALE_LIMIT"
%token NOWALL "NOWALL"
%token GLOBAL "GLOBAL"
%token CONTENT "CONTENT"
%token C1 "C1"
%token C2 "C2"
%token C3 "C3"
%token CONVEX "CONVEX"
%token ENERGY "ENERGY"
%token INFO_ONLY "INFO_ONLY"
%token CONSERVED "CONSERVED"
%token E1 "E1"
%token E2 "E2"
%token E3 "E3"
%token FUNCTION "FUNCTION"
%token PROCEDURE "PROCEDURE"
%token NONNEGATIVE "NONNEGATIVE"
%token NONPOSITIVE "NONPOSITIVE"
%token FORMULA "FORMULA"
%token <m_id> CONSTRAINT "CONSTRAINT"
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
%token <m_id> ACTUAL "ACTUAL"
%token <m_id> AREA "AREA"
%token <m_id> VOLCONST "VOLCONST"
%token <m_id> ACTUAL_VOLUME "ACTUAL_VOLUME"
%token <m_id> LAGRANGE_MULTIPLIER "LAGRANGE_MULTIPLIER"
%token <m_id> CENTEROFMASS "CENTEROFMASS"
%token <m_id> PRESSURE "PRESSURE"
%token <m_id> CONSTRAINTS "CONSTRAINTS"
%token <m_id> DENSITY "DENSITY"
%token <m_id> TENSION "TENSION"
%token CLIP_COEFF "CLIP_COEFF"
%token SLICE_COEFF "SLICE_COEFF"
%token AREA_NORMALIZATION "AREA_NORMALIZATION"
%token MODULUS "MODULUS"
%token SUPPRESS_WARNING "SUPPRESS_WARNING"
%token VALUE "VALUE"

 // terminal symbols
%token SPACE   /* Only transmited if ParsingData::IsSpaceSignificant */
%token <m_int> INTEGER_VALUE
%token <m_real> REAL_VALUE
%token <m_id> IDENTIFIER
%token <m_id> ATTRIBUTE_ID
%token <m_id> METHOD_OR_QUANTITY_ID
%token <m_id> END_COMMENT "*/"
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
 /* method instance value */
%token <m_id> '.'

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
%type <m_node> non_const_expr
%type <m_node> method_expr
%type <m_real> method_value
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
%type <m_int> edge_midpoint
%type <m_id> constraints
%type <m_id> tension_or_density
%type <m_id> pressure_or_lagrange_multiplier
%type <m_attribute> array_rest
%type <m_attribute> array_initializer
%type <m_attribute> array_initializer_list
%type <m_sizetList> array_dimensions_or_indexes;
%type <m_sizet> array_dimension_or_index;

%{
#include "Attribute.h"
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
ExpressionTree* uminusTree (ParsingData& parsingData, ExpressionTree* expr);

%}

%%
datafile
: nlstar header
vertices
{
    //foam.GetParsingData ().PrintTimeCheckpoint ("After vertices:");
}
edges
{
    //foam->GetParsingData ().PrintTimeCheckpoint ("After edges:");
}
faces
{
    //foam->GetParsingData ().PrintTimeCheckpoint ("After faces:");
}
bodies
{
    foam->Preprocess ();
    //foam->GetParsingData ().PrintTimeCheckpoint ("After bodies:");
}

header
: /* empty */
| header dimensionality nlplus
| header space_dimension  nlplus
| header parameter_declaration nlplus
| header attribute nlplus
| header representation nlplus
| header scale_factor nlplus
| header scale_limit nlplus
| header total_time nlplus
| header temperature nlplus
| header constraint_tolerance nlplus
| header SYMMETRIC_CONTENT nlplus
| header KEEP_ORIGINALS nlplus
| header view_matrix nlplus
| header clip_coefficients nlplus
| header slice_coefficient nlplus
| header constraint
| header torus_domain nlplus
| header torus_display_periods nlplus
| header torus_display_origin nlstar
| header length_method_name nlplus
| header area_method_name nlplus
| header quantity nlstar
| header method_instance
| header function_declaration nlplus
| header procedure_declaration nlplus
| header view_transform_generators
| header integral_order_1d nlplus
| header toggle nlplus
| header suppress_warning nlplus
| header array_declaration nlplus
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

space_star
: /* empty */
| SPACE
;

suppress_warning
: SUPPRESS_WARNING INTEGER_VALUE
;

array_declaration
: DEFINE IDENTIFIER number_type array_dimensions_or_indexes array_rest
{
    AttributeArrayAttribute* array = static_cast<AttributeArrayAttribute*> ($5);
    if (array == 0)
    {
	array = AttributeArrayAttribute::NewArray ($4);
    }
    else
	array->CheckDimensions ($4);
    foam->GetParsingData ().SetArray($2, array);    
}
;

array_dimensions_or_indexes
: array_dimension_or_index
{
    $$ = new vector<size_t> (1, $1);
}
| array_dimensions_or_indexes array_dimension_or_index
{
    vector<size_t>* v = $1;
    v->push_back ($2);
    $$ = v;
}
;

array_dimension_or_index
: '[' INTEGER_VALUE ']'
{
    $$ = $2;
}
;

array_rest
: /* empty */
{
    $$ = 0;
}
| '=' nlstar array_initializer
{
    $$ = $3;
}
;

array_initializer
: number
{
    $$ = new RealAttribute ($1);
}
| '{' array_initializer_list '}'
{
    $$ = $2;
}
;

array_initializer_list
: /* empty */
{
    $$ = new AttributeArrayAttribute ();
}
| array_initializer
{
    $$ = new AttributeArrayAttribute ($1);
}
| array_initializer_list ',' nlstar array_initializer
{
    static_cast<AttributeArrayAttribute*>($1)->AddElement ($4);
    $$ = $1;
}
;

toggle
: AREA_NORMALIZATION


integral_order_1d
: INTEGRAL_ORDER_1D colon_assignment INTEGER_VALUE
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
: view_transform_generators_matrices
  {foam->GetParsingData ().SetSpaceSignificant (true);}
  view_transform_generators_matrix
  {foam->GetParsingData ().SetSpaceSignificant (false);}
  nlplus
| {foam->GetParsingData ().SetSpaceSignificant (true);}
  view_transform_generators_matrix
  {foam->GetParsingData ().SetSpaceSignificant (false);}
  nlplus
;

/* 2D or 3D */
view_transform_generators_matrix
: const_expr SPACE const_expr SPACE const_expr space_star nlplus
  const_expr SPACE const_expr SPACE const_expr space_star nlplus
  const_expr SPACE const_expr SPACE const_expr space_star
| const_expr SPACE const_expr SPACE const_expr SPACE const_expr space_star nlplus
  const_expr SPACE const_expr SPACE const_expr SPACE const_expr space_star nlplus
  const_expr SPACE const_expr SPACE const_expr SPACE const_expr space_star nlplus
  const_expr SPACE const_expr SPACE const_expr SPACE const_expr space_star
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
	foam->GetParsingData ().AddMethodOrQuantity ($2);
}

quantity_rest
: ';'
{
    $$ = 0;
}
| quantity_type quantity_lagrange_multiplier quantity_modulus
  quantity_method_list_or_function
{
    $$ = 1;
}

quantity_type
: ENERGY
| FIXED '=' REAL_VALUE
| INFO_ONLY
| CONSERVED
;


quantity_lagrange_multiplier
: /*empty*/
| LAGRANGE_MULTIPLIER number
;

quantity_modulus
: /* empty */
| MODULUS const_expr


quantity_method_list_or_function
: quantity_method_list
| quantity_function
;

quantity_method_list
: quantity_method_list method
| method
;

quantity_function
: FUNCTION method_expr
;

method_instance:
METHOD_INSTANCE IDENTIFIER method_instance_rest
{
    if ($3 != 0)
	foam->GetParsingData ().AddMethodOrQuantity ($2);
}

method_instance_rest
: ';' nlplus
{
    $$ = 0;
}
| nlstar method nlstar
{
    $$ = 1;
}

method
: METHOD METHOD_OR_QUANTITY_ID method_global nlstar method_parameters
;


method_global
: /* empty */
| GLOBAL
;


method_parameters
: /* empty */
| SCALAR_INTEGRAND colon_assignment 
  {foam->GetParsingData ().SetKeywordsIgnored (true);}
  non_const_expr
  {foam->GetParsingData ().SetKeywordsIgnored (false);}
;

function_declaration
: FUNCTION function_return_type IDENTIFIER '('
  function_parameters ')' ';'
;

procedure_declaration
: PROCEDURE IDENTIFIER '(' function_parameters ')' ';'
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

parameter_declaration: PARAMETER IDENTIFIER '=' const_expr
{
    double v = $4;
    foam->GetParsingData ().SetVariable($2, v);
}


attribute
: DEFINE element_type ATTRIBUTE IDENTIFIER attribute_value_type
{
    boost::shared_ptr<AttributeCreator> ac ($5);
    foam->AddAttributeInfo ($2, $4, ac);
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
{
    foam->SetQuadratic (true);
}
| SIMPLEX_REPRESENTATION
;

colon_assignment
: /* empty */
| ':'
;

scale_factor
: SCALE colon_assignment const_expr scale_factor_rest
;

scale_factor_rest
: /* empty */
| FIXED
;

scale_limit
: SCALE_LIMIT colon_assignment const_expr
;

total_time
: TOTAL_TIME colon_assignment const_expr
;

temperature
: TEMPERATURE colon_assignment const_expr
;


constraint_tolerance
: CONSTRAINT_TOLERANCE colon_assignment REAL_VALUE
;

space_dimension
: SPACE_DIMENSION const_expr
{
    foam->SetDimension ($2);
}

/* 2D or 3D */
view_matrix
: VIEW_MATRIX nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr const_expr
{
    foam->SetViewMatrix ( $3,  $4,  $5,  $6,
			 $8,  $9, $10, $11,
			$13, $14, $15, $16,
			$18, $19, $20, $21);
}
| VIEW_MATRIX nlplus
  const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr nlplus
  const_expr const_expr const_expr
{
    foam->SetViewMatrix ( $3,  $4,  $5, 0,
			 $7,  $8,  $9, 0,
			$11, $12, $13, 0,
			  0,   0,   0, 0);
}

clip_coefficients
: CLIP_COEFF '=' '{' clip_coefficient clip_coefficient_list '}'


clip_coefficient
: '{' number ',' number ',' number clip_coefficient_rest

slice_coefficient
: SLICE_COEFF '=' '{' number ',' number ',' number slice_coefficient_rest


clip_coefficient_rest
: ',' number '}'
| '}'

slice_coefficient_rest
: ',' number '}'
| '}'



clip_coefficient_list
: /* empty */
| clip_coefficient_list ',' clip_coefficient


constraint
: CONSTRAINT INTEGER_VALUE constraint_params nlplus
  constraint_type ':' non_const_expr nlplus
  constraint_energy
  constraint_content
{
    foam->GetParsingData ().SetConstraint ($2 - 1, $7);
}
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
{
    delete $5;
}
;

constraint_energy_rest
: /* empty */
| E2 ':' non_const_expr nlplus
  E3 ':' non_const_expr nlplus
{
    delete $3;
    delete $7;
}
;

/* 2D or 3D */
constraint_content
: /* empty */
| CONTENT nl
  C1 ':' non_const_expr nlplus constraint_content_c2c3
{
    delete $5;
}
;

constraint_content_c2c3
: /* empty */
| C2 ':' non_const_expr nlplus constraint_content_c3
{
    delete $3;
}
;

constraint_content_c3
: /* empty */
| C3 ':' non_const_expr nlplus
{
    delete $3;
}
;


const_expr
: expr
{
    double v = $1->Value ();
    $$ = v;
    delete $1;
}

method_value
: METHOD_OR_QUANTITY_ID method_value_rest
{
    $$ = 0;
}

method_value_rest
: /* empty */
| '.' VALUE
;


/* expressions */
/* ====================================================================== */
/* differs from expr in respect to the ATTRIBUTE_ID rule */
non_const_expr
: number
{
    $$ = new ExpressionTreeNumber (foam->GetParsingData (), $1);
}
| IDENTIFIER
{
    $$ = new ExpressionTreeVariable (foam->GetParsingData (), $1);
}
/* Array values */
| IDENTIFIER array_dimensions_or_indexes
{
    $$ = new ExpressionTreeArrayElement (foam->GetParsingData (), $1, *$2);
}
| ATTRIBUTE_ID
{
    $$ = new ExpressionTreeVariable (foam->GetParsingData (), $1);
}
/* Function calls */
| IDENTIFIER '(' non_const_expr ')'
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $3);
}
| IDENTIFIER '(' non_const_expr ',' non_const_expr ')'
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $1, $3, $5);
}
/* Arithmetic operations */
| '-' non_const_expr  %prec UMINUS
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| non_const_expr '+' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr '-' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}

| non_const_expr '*' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr '/' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr '^' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Comparisions */
| non_const_expr '>' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr GE non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr '<' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr LE non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Logical operations */
| '!' non_const_expr
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| non_const_expr AND non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr OR non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Other expressions */
| '(' non_const_expr ')'
{
    $$ = $2;
}
| non_const_expr '=' non_const_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| non_const_expr '?' non_const_expr ':' non_const_expr
{
    $$ = new ExpressionTreeConditional (foam->GetParsingData (), $1, $3, $5);
}

/* differs from expr in respect to the method_value rule */
method_expr
: number
{
    $$ = new ExpressionTreeNumber (foam->GetParsingData (), $1);
}
| IDENTIFIER
{
    $$ = new ExpressionTreeVariable (foam->GetParsingData (), $1);
}
/* Array values */
| IDENTIFIER array_dimensions_or_indexes
{
    $$ = new ExpressionTreeArrayElement (foam->GetParsingData (), $1, *$2);
}
| method_value
{
    $$ = new ExpressionTreeNumber (foam->GetParsingData (), $1);
}
/* Function calls */
| IDENTIFIER '(' method_expr ')'
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $3);
}
| IDENTIFIER '(' method_expr ',' method_expr ')'
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $1, $3, $5);
}
/* Arithmetic operations */
| '-' method_expr  %prec UMINUS
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| method_expr '+' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr '-' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}

| method_expr '*' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr '/' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr '^' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Comparisions */
| method_expr '>' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr GE method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr '<' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr LE method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Logical operations */
| '!' method_expr
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| method_expr AND method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr OR method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Other expressions */
| '(' method_expr ')'
{
    $$ = $2;
}
| method_expr '=' method_expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| method_expr '?' method_expr ':' method_expr
{
    $$ = new ExpressionTreeConditional (foam->GetParsingData (), $1, $3, $5);
}

expr
: number
{
    $$ = new ExpressionTreeNumber (foam->GetParsingData (), $1);
}
| IDENTIFIER
{
    $$ = new ExpressionTreeVariable (foam->GetParsingData (), $1);
}
/* Array values */
| IDENTIFIER array_dimensions_or_indexes
{
    $$ = new ExpressionTreeArrayElement (foam->GetParsingData (), $1, *$2);
}
/* Function calls */
| IDENTIFIER '(' expr ')'
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $3);
}
| IDENTIFIER '(' expr ',' expr ')'
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $1, $3, $5);
}
/* Arithmetic operations */
| '-' expr  %prec UMINUS
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| expr '+' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr '-' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}

| expr '*' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr '/' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr '^' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Comparisions */
| expr '>' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr GE expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr '<' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr LE expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Logical operations */
| '!' expr
{
    $$ = new ExpressionTreeUnaryFunction (foam->GetParsingData (), $1, $2);
}
| expr AND expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr OR expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
/* Other expressions */
| '(' expr ')'
{
    $$ = $2;
}
| expr '=' expr
{
    $$ = new ExpressionTreeBinaryFunction (foam->GetParsingData (), $2, $1, $3);
}
| expr '?' expr ':' expr
{
    $$ = new ExpressionTreeConditional (foam->GetParsingData (), $1, $3, $5);
}

/* end expressions */
/* ====================================================================== */

number
: INTEGER_VALUE
{
    $$ = $1;
}
| REAL_VALUE
{
    $$ = $1;
}

number_type
: INTEGER_TYPE
| REAL_TYPE
;

torus_domain
: torus_type nlplus torus_periods
;

torus_type
: TORUS
| TORUS_FILLED
;

/* 2D */
torus_periods
: PERIODS nl
  const_expr const_expr nl
  const_expr const_expr
{
    using G3D::Vector3;
    foam->SetTorusDomain (Vector3 ($3, $4, 0), Vector3 ($6, $7, 0));
}
/* 3D */
| PERIODS nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr
{
    using G3D::Vector3;
    foam->SetTorusDomain (Vector3 ($3, $4, $5),
			  Vector3 ($7, $8, $9),
			  Vector3 ($11, $12, $13));
}

torus_display_periods
: DISPLAY_PERIODS nl
  const_expr const_expr nl
  const_expr const_expr
| DISPLAY_PERIODS nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr nl
  const_expr const_expr const_expr
;

torus_display_origin
: DISPLAY_ORIGIN nl number nl number nl torus_display_origin_rest
;

torus_display_origin_rest
: /* empty */
| number
;

vertices
: VERTICES nlplus vertex_list
;

/* 2D or 3D */
vertex_list
: /* empty */
| vertex_list INTEGER_VALUE
  {foam->GetParsingData ().SetSpaceSignificant (true);}
  SPACE const_expr SPACE const_expr space_star vertex_list_rest
  {foam->GetParsingData ().SetSpaceSignificant (false);}
  vertex_attribute_list nlplus
{
    vector<NameSemanticValue*>* nameSemanticValueList =
	$11;
    foam->GetParsingData ().SetVertex (
	intToUnsigned($2- 1,
		      "Semantic error: vertex index less than 0: "),
	$5, $7, $9, *nameSemanticValueList,
	foam->GetAttributesInfoElements ().GetInfoVertex ());
    if (nameSemanticValueList != 0)
	NameSemanticValue::DeleteVector(nameSemanticValueList);
}


vertex_list_rest
: /* empty */
/* 2D Z value */
{
    $$ = Foam::Z_COORDINATE_2D;
}
|
/* 3D Z value */
const_expr space_star
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
    $$ = new NameSemanticValue ($1, $2);
}
| FIXED
{
    $$ = 0;
}
| constraints integer_list
{
    $$ = new NameSemanticValue ($1, $2);
}

constraints
: CONSTRAINT
{
    $$ = foam->GetParsingData ().CreateIdentifier ("CONSTRAINTS");
}
| CONSTRAINTS
{
    $$ = $1;
}


user_attribute
: ATTRIBUTE_ID INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1, $2);
}
| ATTRIBUTE_ID REAL_VALUE
{
    $$ =
	new NameSemanticValue ($1, $2);
}
| ATTRIBUTE_ID '{' comma_integer_list '}'
{
    $$ =
	new NameSemanticValue ($1, $3);
}
| ATTRIBUTE_ID '{' comma_real_list '}'
{
    $$ =
	new NameSemanticValue ($1, $3);
}

edges
: EDGES nlplus edge_list
;

edge_list
: /* empty */
| edge_list INTEGER_VALUE INTEGER_VALUE INTEGER_VALUE edge_midpoint
  signs_torus_model edge_attribute_list nlplus
{
    foam->GetParsingData ().SetEdge (
	intToUnsigned($2 - 1, "Semantic error: edge index less than 0: "),
	intToUnsigned($3 - 1, "Semantic error: edge begin less than 0: "),
	intToUnsigned($4 - 1, "Semantic error: edge end less than 0: "),
	intToUnsigned($5 - 1, "Semantic error: edge midpoint less than 0: "),
	*$6,
	*$7,
	foam->GetAttributesInfoElements ().GetInfoEdge (),
	foam->IsQuadratic ());
    delete $6;
    NameSemanticValue::DeleteVector($7);
}

edge_midpoint
: /* empty */
{
    $$ = numeric_limits<int> ().max ();
}
| INTEGER_VALUE
{
    $$ = $1;
}

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
    $$ = new NameSemanticValue ($1, $2);
}
| COLOR color_name
{
    $$ = new NameSemanticValue ($1, $2);
}
| constraints integer_list
{
    $$ = new NameSemanticValue ($1, $2);
}
| tension_or_density const_expr
{
    $$ = new NameSemanticValue ($1, $2);
}
| FIXED
{
    $$ = 0;
}
| NO_REFINE
{
    $$ = 0;
}

tension_or_density
: TENSION
{
    $$ = foam->GetParsingData ().CreateIdentifier ("DENSITY");
}
| DENSITY
{
    $$ = $1;
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
    $$ = Edge::LocationCharToNumber($1[0]);
}
| '*'
{
    $$ = Edge::LocationCharToNumber($1[0]);
}
| '-'
{
    $$ = Edge::LocationCharToNumber($1[0]);
}
;

faces
: FACES nlplus face_list
;

face_list
: /* empty */
| face_list INTEGER_VALUE integer_list face_attribute_list nlplus
{
    vector<int>* intList = $3;
    foam->GetParsingData ().SetFace (
	intToUnsigned($2- 1, "Semantic error: face index less than 0"),
	*intList, *$4,
	foam->GetAttributesInfoElements ().GetInfoFace ());
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
    $$ = new NameSemanticValue ($1, $2);
}
| AREA number END_COMMENT
{
    $$ = new NameSemanticValue ($1, $2);
}
| constraints integer_list
{
    $$ = new NameSemanticValue ($1, $2);
}
| ORIGINAL INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1, $2);
}
| tension_or_density const_expr
{
    $$ = new NameSemanticValue ($1, $2);
}
| FIXED
{
    $$ = 0;
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
: /* empty */
| BODIES nlplus body_list
;

body_list
: /* empty */
/* hack to keep evolver from reusing bubble IDs */
| body_list INTEGER_VALUE nlplus
| body_list INTEGER_VALUE integer_list body_attribute_list nlplus
{
    vector<int>* intList = $3;
    foam->SetBody (
	intToUnsigned($2- 1, "Semantic error: body index less than 0"),
	*intList, *$4, foam->GetParsingData ().OriginalUsed ());
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
    vector<double>* realList = $1;
    realList->push_back ($3);
    $$ = realList;
}
| REAL_VALUE
{
    $$ = new vector<double>(1, $1);
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
    $$ = new NameSemanticValue ($1, $2);
}
| ACTUAL ':' number END_COMMENT
{
    $$ = new NameSemanticValue ($1, $3);
}
| pressure_or_lagrange_multiplier number
{
    $$ = new NameSemanticValue ($1, $2);
}
| ORIGINAL INTEGER_VALUE
{
    $$ = new NameSemanticValue ($1, $2);
}
| VOLCONST number
{
    $$ = new NameSemanticValue ($1, $2);
}
| ACTUAL_VOLUME number
{
    $$ = new NameSemanticValue ($1, $2);
}
| CENTEROFMASS
{
    $$ = 0;
}


pressure_or_lagrange_multiplier
: LAGRANGE_MULTIPLIER
{
    $$ = $1;
}
| PRESSURE
{
    $$ = foam->GetParsingData ().CreateIdentifier ("LAGRANGE_MULTIPLIER");
}


%%

void
EvolverData::parser::error (const EvolverData::parser::location_type& l,
                            const std::string& m)
{
    foam->GetParsingData ().PrintError (l, m.c_str ());
}


size_t intToUnsigned (int i, const char* message)
{
    RuntimeAssert (i >= 0, message, i);
    return static_cast<size_t>(i);
}

ExpressionTree* uminusTree (ParsingData& parsingData, ExpressionTree* expr)
{
    const char* uminusId = parsingData.CreateIdentifier ("-");
    return new ExpressionTreeUnaryFunction (parsingData, uminusId, expr);
}
/** @endcond */

// Local Variables:
// mode: c++
// End:
