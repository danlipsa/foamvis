#include <sstream>
#include "Attribute.h"
#include "AttributeCreator.h"
#include "SemanticError.h"
using namespace std;

Attribute* IntegerAttributeCreator::operator() (SemanticValue& value)
{
    return new IntegerAttribute (value.r);
}

Attribute* RealAttributeCreator::operator() (SemanticValue& value)
{
    return new RealAttribute (value.r);
}


Attribute* IntegerArrayAttributeCreator::operator() (SemanticValue& value)
{
    if (value.numberList->size () != m_size)
    {
	ostringstream ostr;
	ostr << "Declared size of integer array attribute differs "
	    "from size of the attribute value: "
	     << m_size << " != " << value.numberList->size() << ends;
	throw SemanticError (ostr.str ());
    }
    return new IntegerArrayAttribute (*value.numberList);
}

Attribute* RealArrayAttributeCreator::operator() (SemanticValue& value)
{
    if (value.numberList->size () != m_size)
    {
	ostringstream ostr;
	ostr << "Declared size of integer array attribute differs "
	    "from size of the attribute value: "
	     << m_size << " != " << value.numberList->size() << ends;
	throw SemanticError (ostr.str ());
    }
    return new RealArrayAttribute (value.numberList);
}
