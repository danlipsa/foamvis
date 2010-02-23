/**
 * @file   AttributeCreator.cpp
 * @author Dan R. Lipsa
 *
 * Member  definitions for  classes that  know how  to  create various
 * kinds of attributes which can be attached to vertices, edges, faces
 * and bodies.
 */
#include "AttributeCreator.h"
#include "SemanticError.h"

Attribute* IntegerAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{

    if (type != NameSemanticValue::INT)
    {
        ostringstream ostr;
        ostr << "Attribute declared with INTEGER type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    return new IntegerAttribute (value.m_int);
}

Attribute* ColorAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::COLOR)
    {
        ostringstream ostr;
        ostr << "Attribute declared with INTEGER type "
            "has value of type " << type << ends;
        throw SemanticError (ostr.str ());
    }
    return new ColorAttribute (value.m_color);
}



Attribute* RealAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    switch (type)
    {
    case NameSemanticValue::REAL:
        return new RealAttribute (value.m_real);
    case NameSemanticValue::INT:
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
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::INT_ARRAY)
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
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    if (type != NameSemanticValue::REAL_ARRAY)
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
