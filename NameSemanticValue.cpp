/**
 * @file   NameSemanticValue.h
 * @author Dan R. Lipsa
 *
 * Definitions for tupple (name, type, value) used for attributes
 */
#include "NameSemanticValue.h"
#include "Utils.h"
#include "Debug.h"

ostream& operator<< (ostream& ostr, 
                     const NameSemanticValue& nameSemanticValue)
{
    ostr << nameSemanticValue.m_name << ", " 
         << nameSemanticValue.m_type << ", ";
    switch (nameSemanticValue.m_type)
    {
    case AttributeType::INT:
        ostr << nameSemanticValue.m_semanticValue.m_int;
        break;
    case AttributeType::REAL:
        ostr << nameSemanticValue.m_semanticValue.m_real;
        break;
    case AttributeType::COLOR:
        ostr << nameSemanticValue.m_semanticValue.m_color;
        break;
    case AttributeType::INT_ARRAY:
        ostr << nameSemanticValue.m_semanticValue.m_intList;
        break;
    case AttributeType::REAL_ARRAY:
        ostr << nameSemanticValue.m_semanticValue.m_realList;
        break;
    default:
	RuntimeAssert (false, "Invalid NameSemanticValue::Type: ",
		       nameSemanticValue.m_type);
    }
    return ostr;
}

vector<NameSemanticValue*>* NameSemanticValue::PushBack (
    vector<NameSemanticValue*>* listSoFar, NameSemanticValue* attribute)
{
    if (listSoFar == 0)
        listSoFar = new vector<NameSemanticValue*>();
    if (attribute != 0)
	listSoFar->push_back (attribute);
    return listSoFar;
}

void NameSemanticValue::DeleteVector (vector<NameSemanticValue*>* v)
{
    if (v != 0)
    {
        for_each (v->begin (), v->end (), bl::delete_ptr ());
        delete v;
    }
}
