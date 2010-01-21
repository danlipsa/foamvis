#include "NameSemanticValue.h"
#include "ElementUtils.h"


ostream& operator<< (ostream& ostr, NameSemanticValue::Type& type)
{
    switch (type)
    {
    case NameSemanticValue::INT:
        ostr << "INT";
        break;
    case NameSemanticValue::REAL:
        ostr << "REAL";
        break;
    case NameSemanticValue::COLOR:
        ostr << "COLOR";
        break;
    case NameSemanticValue::INT_ARRAY:
        ostr << "INT_ARRAY";
        break;
    case NameSemanticValue::REAL_ARRAY:
        ostr << "REAL_ARRAY";
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
    case NameSemanticValue::INT:
        ostr << nameSemanticValue.m_semanticValue.m_int;
        break;
    case NameSemanticValue::REAL:
        ostr << nameSemanticValue.m_semanticValue.m_real;
        break;
    case NameSemanticValue::COLOR:
        ostr << nameSemanticValue.m_semanticValue.m_color;
        break;
    case NameSemanticValue::INT_ARRAY:
        ostr << nameSemanticValue.m_semanticValue.m_intList;
        break;
    case NameSemanticValue::REAL_ARRAY:
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
        for_each (v->begin (), v->end (), DeleteElementPtr<NameSemanticValue>);
        delete v;
    }
}
