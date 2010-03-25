/**
 * @file   SemanticType.h
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Types of attributes
 */

#include "SemanticType.h"
#include "Debug.h"

ostream& operator<< (ostream& ostr, SemanticType::Name type)
{
    switch (type)
    {
    case SemanticType::INT:
        ostr << "INT";
        break;
    case SemanticType::REAL:
        ostr << "REAL";
        break;
    case SemanticType::COLOR:
        ostr << "COLOR";
        break;
    case SemanticType::INT_ARRAY:
        ostr << "INT_ARRAY";
        break;
    case SemanticType::REAL_ARRAY:
        ostr << "REAL_ARRAY";
        break;
    default:
	RuntimeAssert (false, "Invalid SemanticType: ", type);
    }
    return ostr;
}
