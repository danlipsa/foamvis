#include <stdexcept>
#include <iostream>
#include <sstream>
#include "NameSemanticValue.h"
#include "ElementUtils.h"

using namespace std;

ostream& operator<< (ostream& ostr, NameSemanticValue::Type& type)
{
    switch (type)
    {
    case NameSemanticValue::INT_SEMANTIC_TYPE:
	ostr << "INT_SEMANTIC_TYPE";
	break;
    case NameSemanticValue::REAL_SEMANTIC_TYPE:
	ostr << "REAL_SEMANTIC_TYPE";
	break;
    case NameSemanticValue::COLOR_SEMANTIC_TYPE:
	ostr << "COLOR_SEMANTIC_TYPE";
	break;
    case NameSemanticValue::INT_ARRAY_SEMANTIC_TYPE:
	ostr << "INT_ARRAY_SEMANTIC_TYPE";
	break;
    case NameSemanticValue::REAL_ARRAY_SEMANTIC_TYPE:
	ostr << "REAL_ARRAY_SEMANTIC_TYPE";
	break;
    default:
    {
	ostringstream sstr;
	sstr << "Invalid NameSemanticValue::Type: " << type << ends;
	throw domain_error (sstr.str ());
    }
    }
    return ostr;
}

ostream& operator<< (ostream& ostr, 
		     NameSemanticValue& nameSemanticValue)
{
    ostr << nameSemanticValue.m_name << ", " 
	 << nameSemanticValue.m_type << ", ";
    switch (nameSemanticValue.m_type)
    {
    case NameSemanticValue::INT_SEMANTIC_TYPE:
	ostr << nameSemanticValue.m_semanticValue.m_int;
	break;
    case NameSemanticValue::REAL_SEMANTIC_TYPE:
	ostr << nameSemanticValue.m_semanticValue.m_real;
	break;
    case NameSemanticValue::COLOR_SEMANTIC_TYPE:
	ostr << nameSemanticValue.m_semanticValue.m_color;
	break;
    case NameSemanticValue::INT_ARRAY_SEMANTIC_TYPE:
	ostr << nameSemanticValue.m_semanticValue.m_intList;
	break;
    case NameSemanticValue::REAL_ARRAY_SEMANTIC_TYPE:
	ostr << nameSemanticValue.m_semanticValue.m_realList;
	break;
    default:
    {
	ostringstream sstr;
	sstr << "Invalid NameSemanticValue::Type: " 
	     << nameSemanticValue.m_type << ends;
	throw domain_error (sstr.str ());
    }
    }
    return ostr;
}

vector<NameSemanticValue*>* NameSemanticValue::PushBack (
    vector<NameSemanticValue*>* listSoFar)
{
    if (listSoFar == 0)
	listSoFar = new vector<NameSemanticValue*>();
    listSoFar->push_back (this);
    return listSoFar;
}

void NameSemanticValue::DeleteVector (vector<NameSemanticValue*>* v)
{
    if (v != 0)
    {
	for_each (v->begin (), v->end (), 
		  DeleteElementPtr<NameSemanticValue>);
	delete v;
    }
}
