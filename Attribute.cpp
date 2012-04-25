/**
 * @file Attribute.cpp
 * @author Dan R. Lipsa
 *
 * Contains definition  of class members for all types  of attributes which
 * can be attached to vertices, edges, faces and bodies.
 */
#include "Attribute.h"
#include "Debug.h"

ostream& IntegerArrayAttribute::Print (ostream& ostr) const 
{
    ostr << "[";
    ostream_iterator<int> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    ostr << "]";
    return ostr;
}

ostream& RealArrayAttribute::Print (ostream& ostr) const
{
    ostr << "[";
    ostream_iterator<double> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    ostr << "]";
    return ostr;
}


// AttributeArrayAttribute
// ======================================================================
AttributeArrayAttribute::AttributeArrayAttribute ()
{
    m_values.reset (new vector< boost::shared_ptr<Attribute> > ());
}

AttributeArrayAttribute::AttributeArrayAttribute (Attribute* element)
{
    m_values.reset (new vector< boost::shared_ptr<Attribute> > (
			1, boost::shared_ptr<Attribute> (element)));
}


AttributeArrayAttribute::AttributeArrayAttribute (size_t n, double value)
{
    m_values.reset (new vector< boost::shared_ptr<Attribute> > (n));
    for (size_t i = 0; i < m_values->size (); ++i)
	(*m_values)[i] = 
	    boost::shared_ptr<Attribute> (new RealAttribute (value));
}

AttributeArrayAttribute::AttributeArrayAttribute (size_t n)
{
    m_values.reset (new vector< boost::shared_ptr<Attribute> > (n));
}

ostream& AttributeArrayAttribute::Print (ostream& ostr) const
{
    ostr << "[";
    vector< boost::shared_ptr<Attribute> >& va = *m_values;
    BOOST_FOREACH (boost::shared_ptr<Attribute> atr, va)
    {
	atr->Print (ostr);
    }
    ostr << "]";
    return ostr;
}

void AttributeArrayAttribute::CheckDimensions (vector<size_t>* dimensions)
{
    checkDimensions (dimensions, 0);
}

AttributeArrayAttribute* AttributeArrayAttribute::NewArray (
    vector<size_t>* dimensions)
{
    return newArray (dimensions, 0);
}


void AttributeArrayAttribute::checkDimensions (vector<size_t>* dimensions,
					       size_t currentDimensionIndex)
{
    size_t dimensionsSize = dimensions->size ();
    if (currentDimensionIndex >= dimensionsSize)
	ThrowException ("Invalid index: ", currentDimensionIndex,
			"should be less than ", dimensionsSize);
    size_t dimension = m_values->size ();
    size_t expectedDimension = (*dimensions)[currentDimensionIndex];
    if (dimension != expectedDimension)
	ThrowException ("Wrong array dimension: ", dimension, 
			" expected: ", expectedDimension);
    if (dimensionsSize - 1 == currentDimensionIndex)
	return;
    BOOST_FOREACH (boost::shared_ptr<Attribute> attribute, *m_values)
    {
	boost::static_pointer_cast<AttributeArrayAttribute> (
	    attribute)->checkDimensions (
		dimensions, currentDimensionIndex + 1);
    }
}

AttributeArrayAttribute* AttributeArrayAttribute::newArray (
    vector<size_t>* dimensions, size_t currentDimensionIndex)
{
    size_t dimensionsSize = dimensions->size ();
    if (currentDimensionIndex >= dimensionsSize)
	ThrowException ("Invalid index: ", currentDimensionIndex,
			"should be less than ", dimensionsSize);
    
    if (currentDimensionIndex == dimensionsSize - 1)
	return new AttributeArrayAttribute (
	    (*dimensions)[currentDimensionIndex], 0);
    else
    {
	AttributeArrayAttribute* array = 
	    new AttributeArrayAttribute ((*dimensions)[currentDimensionIndex]);
	for (size_t i = 0; i < array->size (); ++i)
	    array->setElement (
		i, newArray (dimensions, currentDimensionIndex + 1));
	return array;
    }
}

double AttributeArrayAttribute::Get (const vector<size_t>& index) const
{
    const AttributeArrayAttribute* current = this;
    size_t i = 0;
    for (; i < index.size () - 1; ++i)
	current = boost::static_pointer_cast<AttributeArrayAttribute> (
	    current->getElement (index[i])).get ();
    return static_cast<double> (
	(*boost::static_pointer_cast<RealAttribute> (
	    current->getElement (index[i]))));
}

ostream& operator<< (ostream& ostr, const Attribute& attribute)
{
    return attribute.Print(ostr);
}

ostream& operator<< (ostream& ostr, const Attribute* attribute)
{
    return ostr << *attribute;
}

