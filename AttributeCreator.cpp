/**
 * @file   AttributeCreator.cpp
 * @author Dan R. Lipsa
 *
 * Member  definitions for  classes that  know how  to  create various
 * kinds of attributes which can be attached to vertices, edges, faces
 * and bodies.
 */
#include "AttributeCreator.h"
#include "Debug.h"

Attribute* IntegerAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    RuntimeAssert (type == NameSemanticValue::INT,
		   "Attribute declared with INTEGER type has value of type ",
		   type);
    return new IntegerAttribute (value.m_int);
}

Attribute* ColorAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    
    RuntimeAssert (type == NameSemanticValue::COLOR,
		   "Attribute declared with INTEGER type has value of type ",
		   type);
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
	RuntimeAssert (false,
		       "Attribute declared with REAL type has value of type ",
		       type);
	return 0;
    }
}

Attribute* IntegerArrayAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    RuntimeAssert (
	type == NameSemanticValue::INT_ARRAY,
	"Attribute declared with INTEGER_ARRAY type has value of type ",
	type);
    RuntimeAssert (
	value.m_intList->size () == m_size,
	"Declared size of integer array attribute differs "
	"from size of the attribute value: ",
	m_size, " != ", value.m_intList->size());
    return new IntegerArrayAttribute (value.m_intList);
}

Attribute* RealArrayAttributeCreator::operator() (
     EvolverData::parser::semantic_type& value, 
    NameSemanticValue::Type type)
{
    RuntimeAssert (
	type == NameSemanticValue::REAL_ARRAY,
	"Attribute declared with REAL_ARRAY type "
	"has value of type ", type);
    RuntimeAssert (
	value.m_realList->size () == m_size,
	"Declared size of integer array attribute differs "
	"from size of the attribute value: ", m_size, " != ", 
	value.m_realList->size());
    return new RealArrayAttribute (value.m_realList);
}
