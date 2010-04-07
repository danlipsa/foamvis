/**
 * @file   ParsingData.h
 * @author Dan R. Lipsa
 *
 * Declaration of ParsingData
 */
#ifndef __PARSING_DATA_H__
#define __PARSING_DATA_H__

#include "Comparisons.h"
#include "ParsingDriver.h"

/**
 * Stores data used during  the parsing such as identifiers, variables
 * and functions.
 */
class ParsingData : public ParsingDriver
{
public:
    /**
     * How are variables stored
     */
    typedef map<const char*, float, LessThanNoCase> Variables;
    /**
     * A unary function
     */
    //typedef float (*UnaryFunction)(float);
    typedef boost::function<float (float)> UnaryFunction;
    /**
     * A binary function
     */
    typedef boost::function<float (float, float)> BinaryFunction;
    /**
     * How are unary functions stored.
     */
    typedef map<const char*, UnaryFunction, 
		LessThanNoCase> UnaryFunctions;
    /**
     * How are binary functions stored
     */
    typedef map<const char*, BinaryFunction, 
                     LessThanNoCase> BinaryFunctions;
    /**
     * How are identifiers stored
     */
    typedef map<const char*, string*, LessThanNoCase> Identifiers;
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
    string* CreateIdentifier(char* id);
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
    friend ostream& operator<< (ostream& ostr, ParsingData& pd);
    /**
     * Used for  profiling. Prints to  the debug stream  a description
     * and the time since the last checkpoint.  
     * @param description what should be printed together with the time
     */
    void PrintTimeCheckpoint (string& description);

private:
    struct BinaryFunctionInformation
    {
	const char* m_name;
	BinaryFunction m_function;
    };
    struct UnaryFunctionInformation
    {
	const char* m_name;
	UnaryFunction m_function;
    };


private:
    /**
     * Stores  variables  read   from  the  datafile  (declared  using
     * PARAMETER keyword in the Evolver DMP file)
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
    /**
     * Used for profiling. Stores the previous time checkpoint
     */
    clock_t m_previousTime;
};

/**
 * Stores semantic values
 */
typedef EvolverData::parser::semantic_type YYSTYPE;
/**
 * Stores location in the parsed file
 */
typedef EvolverData::parser::location_type YYLTYPE;
/**
 * Prototype for lexing function
 */
# define YY_DECL int \
    EvolverDatalex (YYSTYPE* yylval_param, YYLTYPE* yylloc_param, \
           void* yyscanner)
/**
 * The lexing function
 * @param yylval_param where to store the value read by the lexer
 * @param yylloc_param where to store the location in the file
 * @param yyscanner opaque object for the scanner
 * @return 0 for success, different then 0 otherwise
 */
YY_DECL;

#endif //__PARSING_DATA_H__

// Local Variables:
// mode: c++
// End:
