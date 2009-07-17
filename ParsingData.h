/**
 * @file   ParsingData.h
 * @author Dan R. Lipsa
 *
 * Declaration of ParsingData
 */
#ifndef __PARSING_DATA_H__
#define __PARSING_DATA_H__
#include <string.h>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

/**
 * Binary function that compares two C strings
 */
struct LessThanCharPtr : binary_function<const char*, const char*, bool>
{
    /**
     * Compares two character pointers
     * @param s1 first C string
     * @param s2 second C string
     * @return true if the first argument is less than the second argument.
     */
    bool operator()(const char* s1, const char* s2) const
    {
	return strcmp(s1, s2) < 0;
    }
};

/**
 * Stores data used during  the parsing such as identifiers, variables
 * and functions.
 */
class ParsingData
{
public:
    ParsingData ();
    ~ParsingData ();
    /**
     * Stores a variable name and value in the Data object
     * @param id  variable name. It  is allocated by the  lexer using
     *         CreateId and it is dealocated by the ParsingData object
     * @param value variable value
     */
    void SetVariable (const char* id, float value)
    {m_variables[id] = value;}
    float GetVariableValue (const char* id);
    string* CreateId(const char* id);
    float (*GetUnaryFunction (const char* name))(float);

    float ExecuteUnaryFunction (const char* name, float value)
    {float (*f)(float) = GetUnaryFunction (name);return f (value);}

    float (*GetBinaryFunction (const char* name))(float, float) 
    {return m_binaryFunctions[name];}

    typedef map<const char*, float, LessThanCharPtr> Variables;
    typedef map<const char*, float (*)(float), LessThanCharPtr> UnaryFunctions;
    typedef map<const char*, 
		float (*)(float, float),LessThanCharPtr> BinaryFunctions;
    typedef map<const char*, string*, LessThanCharPtr> Identifiers;
    friend ostream& operator<< (ostream& ostr, ParsingData& pd);
private:
    Variables m_variables;
    UnaryFunctions m_unaryFunctions;
    BinaryFunctions m_binaryFunctions;
    Identifiers m_identifiers;
};

#endif

// Local Variables:
// mode: c++
// End:
