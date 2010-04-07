/**
 * @file   ParsingData.cpp
 * @author Dan R. Lipsa
 *
 * Definition of the ParsingData class
 */

#include "Debug.h"
#include "DebugStream.h"
#include "ParsingData.h"

/**
 * Throws  an  exception  because  we  should  not  have
 * assignments in constant expressions.
 * @return it throws an exception before returning.
 */
static float assignmentFunction (float, float)
{
    throw logic_error ("Assignment operation in constant expression");
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
{
    delete pair.second;
}


ostream& operator<< (ostream& ostr, ParsingData& pd)
{
    ostr << "Variables: " << endl;
    for_each (pd.m_variables.begin (), pd.m_variables.end (),
              printVariable (ostr));
    return ostr;
}


ParsingData::ParsingData ()
{
    BinaryFunctionInformation BINARY_FUNCTION_INFORMATION[] = 
    {
	{"+", plus<float> ()},
	{"-", minus<float> ()},
	{"*", multiplies<float> ()},
	{"/", divides<float> ()},
	{"^", powf},
	{"=", assignmentFunction},
	{"atan2", atan2f},
	{">=", greater_equal<float>()}
    };
    UnaryFunctionInformation UNARY_FUNCTION_INFORMATION[] =
    {
	{"-", negate<float> ()},
	{"sqrt", sqrtf}
    };

    BOOST_FOREACH (BinaryFunctionInformation bfi, BINARY_FUNCTION_INFORMATION)
	m_binaryFunctions[bfi.m_name] = bfi.m_function;
    BOOST_FOREACH (UnaryFunctionInformation ufi, UNARY_FUNCTION_INFORMATION)
	m_unaryFunctions[ufi.m_name] = ufi.m_function;

    m_previousTime = clock ();
}

ParsingData::~ParsingData ()
{
    for_each(m_identifiers.begin (), m_identifiers.end (), deleteIdentifier);
}

float ParsingData::GetVariableValue (const char* id) 
{
    Variables::iterator it = m_variables.find (id);
    RuntimeAssert (it != m_variables.end (), "Undeclared variable: ", id);
    return it->second;
}

ParsingData::UnaryFunction ParsingData::GetUnaryFunction (const char* name)
{
    UnaryFunctions::iterator it = m_unaryFunctions.find (name);
    RuntimeAssert (it != m_unaryFunctions.end (),
		   "Invalid unary function name: ", name);
    return it->second;
}

ParsingData::BinaryFunction ParsingData::GetBinaryFunction (const char* name)
{
    BinaryFunctions::iterator it = m_binaryFunctions.find (name);
    RuntimeAssert (it != m_binaryFunctions.end (),
		   "Invalid binary function name: ", name);
    return it->second;
}



string* ParsingData::CreateIdentifier(char* id)
{
    Identifiers::iterator it = m_identifiers.find (id);
    if (it == m_identifiers.end ())
    {
        string* stringId = new string(id);
        // do not store id, as it comes from the parser and it will go away.
        m_identifiers[stringId->c_str ()] = stringId;
        return stringId;
    }
    else
        return it->second;
}

void ParsingData::PrintTimeCheckpoint (string& description)
{
    clock_t time = clock ();
	cdbg << description << ": " 
		<< static_cast<float>(time - m_previousTime) / CLOCKS_PER_SEC
		<< " sec" << endl;
	m_previousTime = time;
}
