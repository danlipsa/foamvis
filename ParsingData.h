/**
 * @file   ParsingData.h
 * @author Dan R. Lipsa
 *
 * Declaration of ParsingData
 */
#ifndef __PARSING_DATA_H__
#define __PARSING_DATA_H__
#include <functional>
#include <string>
#include <map>
#include "ElementUtils.h"
#include "ParserDriver.h"

/**
 * Stores data used during  the parsing such as identifiers, variables
 * and functions.
 */
class ParsingData : public ParserDriver
{
public:
    /**
     * How are variables stored
     */
    typedef std::map<const char*, float, LessThanNoCase> Variables;
    /**
     * A unary function
     */
    typedef float (*UnaryFunction)(float);
    /**
     * A binary function
     */
    typedef float (*BinaryFunction)(float, float);
    /**
     * How are unary functions stored.
     */
    typedef std::map<const char*, UnaryFunction, LessThanNoCase> UnaryFunctions;
    /**
     * How are binary functions stored
     */
    typedef std::map<const char*, BinaryFunction,LessThanNoCase> BinaryFunctions;
    /**
     * How are identifiers stored
     */
    typedef std::map<const char*, std::string*, LessThanNoCase> Identifiers;
    /**
     * Constructs a ParsingData object
     */
    ParsingData ();
    /**
     * Destructs this object
     */
    ~ParsingData ();
    /**
     * Stores a variable name and value in the ParsingData object
     * @param id  variable name. It  is allocated by the  lexer using
     *         CreateIdentifier and it is dealocated by the ParsingData object
     * @param value variable value
     */
    void SetVariable (const char* id, float value) {m_variables[id] = value;}
    /**
     * Retrieves a variable value
     * @param name variable name
     * @return variable value
     */
    float GetVariableValue (const char* name);
    /**
     * Stores a string from the lexer for later use in the parser
     * @param id string from the lexer
     * @return a string pointer which is stored in ParsingData object
     */
    std::string* CreateIdentifier(const char* id);
    /**
     * Returns the unary function with the name supplied by the parameter
     * @param name name of the function to be retrieved
     * @return a unary function
     */
    UnaryFunction GetUnaryFunction (const char* name);
    /**
     * Returns the binary function with the name supplied by the parameter
     * @param name name of the function to be retrieved
     * @return a binary function
     */
    BinaryFunction GetBinaryFunction (const char* name);
    /**
     * Pretty prints the ParsingData object
     * @param ostr output stream where to print the object
     * @param pd object to be printed
     */    
    friend std::ostream& operator<< (std::ostream& ostr, ParsingData& pd);
private:
    /**
     * Stores  variables  read   from  the  datafile  (declared  using
     * PARAMETER keyword)
     */
    Variables m_variables;
    /**
     * Unary functions
     */
    UnaryFunctions m_unaryFunctions;
    /**
     * Binary functions
     */
    BinaryFunctions m_binaryFunctions;
    /**
     * Identifiers
     */
    Identifiers m_identifiers;
};

// define types used in flex
typedef EvolverData::parser::semantic_type YYSTYPE;
typedef EvolverData::parser::location_type YYLTYPE;
// Announce to Flex the prototype we want for lexing function, ...
# define YY_DECL int \
    EvolverDatalex (YYSTYPE* yylval_param, YYLTYPE* yylloc_param, \
           void* yyscanner)
// ... and declare it for the parser's sake.
YY_DECL;

#endif //__PARSING_DATA_H__

// Local Variables:
// mode: c++
// End:
