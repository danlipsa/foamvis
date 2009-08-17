#include <sstream>
#include "Attribute.h"
#include "AttributeCreator.h"
#include "SemanticError.h"
#include "NameSemanticValue.h"
using namespace std;

Attribute* IntegerAttributeCreator::operator() (const SemanticValue& value, 
                                                NameSemanticValue::Type type)
{

    if (type != NameSemanticValue::INT_SEMANTIC_TYPE)
    {
        ostringstream ostr;
        ostr << "Attribute declared with INTEGER type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    return new IntegerAttribute (value.m_int);
}

Attribute* ColorAttributeCreator::operator() (const SemanticValue& value, 
                                              NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::COLOR_SEMANTIC_TYPE)
    {
        ostringstream ostr;
        ostr << "Attribute declared with INTEGER type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    return new ColorAttribute (value.m_color);
}



Attribute* RealAttributeCreator::operator() (const SemanticValue& value, 
                                             NameSemanticValue::Type type)
{
    switch (type)
    {
    case NameSemanticValue::REAL_SEMANTIC_TYPE:
        return new RealAttribute (value.m_real);
    case NameSemanticValue::INT_SEMANTIC_TYPE:
        return new RealAttribute (value.m_int);
    default:
    {
        ostringstream ostr;
        ostr << "Attribute declared with REAL type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    }
}


Attribute* IntegerArrayAttributeCreator::operator() (
    const SemanticValue& value, 
    NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::INT_ARRAY_SEMANTIC_TYPE)
    {
        ostringstream ostr;
        ostr << "Attribute declared with INTEGER_ARRAY type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    if (value.m_intList->size () != m_size)
    {
        ostringstream ostr;
        ostr << "Declared size of integer array attribute differs "
            "from size of the attribute value: "
             << m_size << " != " << value.m_intList->size() << ends;
        throw SemanticError (ostr.str ());
    }
    return new IntegerArrayAttribute (value.m_intList);
}

Attribute* RealArrayAttributeCreator::operator() (
    const SemanticValue& value, 
    NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::REAL_ARRAY_SEMANTIC_TYPE)
    {
        ostringstream ostr;
        ostr << "Attribute declared with REAL_ARRAY type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    if (value.m_realList->size () != m_size)
    {
        ostringstream ostr;
        ostr << "Declared size of integer array attribute differs "
            "from size of the attribute value: "
             << m_size << " != " << value.m_realList->size() << ends;
        throw SemanticError (ostr.str ());
    }
    return new RealArrayAttribute (value.m_realList);
}
