#include <math.h>
#include "Element.h"
#include "ParsingData.h"
#include "SemanticError.h"

static float negateFunction (float v)
{return -v;}
static float plusFunction (float first, float second)
{return first + second;}
static float minusFunction (float first, float second)
{return first - second;}
static float dividesFunction (float first, float second)
{return first / second;}
static float multipliesFunction (float first, float second)
{return first * second;}
static float assignmentFunction (float first, float second)
{
    throw SemanticError ("Assignment operation in constant expression");
}


struct printVariable : unary_function<pair<const char*, float>, void>
{
    printVariable (ostream& ostr) : m_ostr(ostr) {}

    void operator() (pair<const char*, float> value)
    {
	m_ostr << value.first << ": " << value.second << endl;
    }
private:
    ostream& m_ostr;
};

struct deleteIdentifier : 
    public unary_function<pair<const char*, string*>, void>
{
    void operator() (pair<const char*, string*> pair)
    {
	delete pair.second;
    }
};

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
    for_each(m_identifiers.begin (), m_identifiers.end (), deleteIdentifier ());
}

float ParsingData::GetVariableValue (const char* id) 
{
    Variables::iterator it = m_variables.find (id);
    if (it == m_variables.end ())
	throw SemanticError (string("Undeclared variable: ") + id);
    else
	return it->second;
}

float (*ParsingData::GetUnaryFunction (const char* name))(float)
{
    UnaryFunctions::iterator it = m_unaryFunctions.find (name);
    if (it == m_unaryFunctions.end ())
	throw SemanticError (string("Invalid unary function name: ") + name);
    else
	return it->second;
}

string* ParsingData::CreateId(const char* id)
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

