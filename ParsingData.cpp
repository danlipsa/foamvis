#include <math.h>
#include "Element.h"
#include "ParsingData.h"
#include "SemanticError.h"

/**
 * Negates a number. Used in the parser expressions.
 * @param v a floating point number
 * @return the negation of the parameter
 */
static float negateFunction (float v) {return -v;}
/**
 * Adds two numbers.Used in the parser expressions.
 * @param first first number in the sum
 * @param second second number in the sum
 * @return the sum of the two parameters
 */
static float plusFunction (float first, float second) {return first + second;}
/**
 * Subtracts two numbers
 * @param first first number
 * @param second second number
 * @return the difference of the two numbers
 */
static float minusFunction (float first, float second)
{return first - second;}
/**
 * Divides two numbers
 * @param first first number
 * @param second second number
 * @return the division of the two numbers
 */
static float dividesFunction (float first, float second)
{return first / second;}
/**
 * Multiplies two numbers
 * @param first first number
 * @param second second number
 * @return the multiplication of the two numbers
 */
static float multipliesFunction (float first, float second)
{return first * second;}
/**
 * Throws  a  SemanticError  exception  because  we  should  not  have
 * assignments in constant expressions.
 * @param first first number
 * @param second second number
 * @return it throws an exception before returning.
 */
static float assignmentFunction (float first, float second)
{
    throw SemanticError ("Assignment operation in constant expression");
}
/**
 * Pretty prints a variable. Used by a for_each algorithm.
 */
struct printVariable : public unary_function<pair<const char*, float>, void>
{
    /**
     * Constructs a printVariable object
     * @param ostr stream where the variable is printed
     */
    printVariable (ostream& ostr) : m_ostr(ostr) {}
    /**
     * Pretty prints a variable
     * @param nameValue a name-value pair
     */
    void operator() (pair<const char*, float> nameValue)
    {
	m_ostr << nameValue.first << ": " << nameValue.second << endl;
    }
private:
    /**
     * Stream where the variable will be printed.
     */
    ostream& m_ostr;
};

/**
 * Deletes an identifier
 * @param  pair  this  is  how  an identifier  is  stored  in  the
 * ParsingData object. We delete the string* part.
 */
inline void deleteIdentifier (pair<const char*, string*> pair)
{delete pair.second;}

ParsingData::ParsingData ()
{
    m_unaryFunctions["-"] = negateFunction;

    m_binaryFunctions["+"] = plusFunction;
    m_binaryFunctions["-"] = minusFunction;
    m_binaryFunctions["*"] = multipliesFunction;
    m_binaryFunctions["/"] = dividesFunction;
    m_binaryFunctions["^"] = powf;
    m_binaryFunctions["="] = assignmentFunction;
}

ParsingData::~ParsingData ()
{
    for_each(m_identifiers.begin (), m_identifiers.end (), deleteIdentifier);
}

float ParsingData::GetVariableValue (const char* id) 
{
    Variables::iterator it = m_variables.find (id);
    if (it == m_variables.end ())
	throw SemanticError (string("Undeclared variable: ") + id);
    else
	return it->second;
}

ParsingData::UnaryFunction ParsingData::GetUnaryFunction (const char* name)
{
    UnaryFunctions::iterator it = m_unaryFunctions.find (name);
    if (it == m_unaryFunctions.end ())
	throw SemanticError (string("Invalid unary function name: ") + name);
    else
	return it->second;
}

ParsingData::BinaryFunction ParsingData::GetBinaryFunction (const char* name)
{
    BinaryFunctions::iterator it = m_binaryFunctions.find (name);
    if (it == m_binaryFunctions.end ())
	throw SemanticError (string("Invalid binary function name: ") + name);
    else
	return it->second;
}



string* ParsingData::CreateIdentifier(const char* id)
{
    Identifiers::iterator it = m_identifiers.find (id);
    if (it == m_identifiers.end ())
    {
	string* stringId = new string(id);
	m_identifiers[id] = stringId;
	return stringId;
    }
    else
	return it->second;
}

ostream& operator<< (ostream& ostr, ParsingData& pd)
{
    ostr << "Variables: " << endl;
    for_each (pd.m_variables.begin (), pd.m_variables.end (),
	      printVariable (ostr));
    return ostr;
}

