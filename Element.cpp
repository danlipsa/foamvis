/**
 * @file Element.cpp
 * @author Dan R. Lipsa
 * 
 * Definitions for methods of the Element class.
 */
#include "Attribute.h"
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Debug.h"
#include "Element.h"
#include "ElementUtils.h"
#include "NameSemanticValue.h"

// Private Classes
// ======================================================================

/**
 * Functor that stores an attribute in an element.
 */
class storeAttribute : 
    public unary_function < NameSemanticValue*, void>
{
public:
    /**
     * Constructor for the functor
     * @param where element where to store the attribute
     * @param infos information about attributes and how are they created
     */
    storeAttribute(
        Element& where, AttributesInfo& infos) : 
        m_where (where), m_infos (infos) {}
    /**
     * Stores an attribute
     * @param nameSemanticValue Name and value of the attribute
     */
    void operator() (NameSemanticValue* nameSemanticValue)
    {
        try
        {
            AttributeInfo* info = 
                m_infos.GetAttributeInfo (nameSemanticValue->GetName ());
            RuntimeAssert (info != 0, 
			   "Attribute \"", nameSemanticValue->GetName (),
			   "\" was not defined");
            size_t index = info->GetIndex ();
            if (index == AttributeInfo::INVALID_INDEX)
                return;
            else
            {
                AttributeCreator& creator = info->GetCreator ();
                m_where.SetAttribute (
                    index, 
		    creator(nameSemanticValue->GetSemanticValue (), 
			    nameSemanticValue->GetType ()));
            }
        }
        catch (exception& e)
        {
            throw logic_error (string(nameSemanticValue->GetName ()) + ": "
			       + e.what ());
        }
    }
private:
    /**
     * Element where the attribute is stored
     */
    Element& m_where;
    /**
     * Information about all attributes and how they are created.
     */
    AttributesInfo& m_infos;
};


// Static Fields
// ======================================================================
const size_t Element::INVALID_INDEX = numeric_limits<size_t>::max ();

// Methods
// ======================================================================

Element::Element (const Element& other) :
    m_id (other.m_id),
    m_status (other.m_status)
{
    if (other.m_attributes.get () != 0)
    {
	m_attributes.reset (new Attributes (other.m_attributes->size ()));
	*m_attributes = *other.m_attributes;
    }
}


void Element::SetAttribute (size_t i, Attribute* attribute)
{
    using boost::shared_ptr;
    if (m_attributes == 0)
    {
	m_attributes.reset (new Attributes ());
    }
    if (i >= m_attributes->size ())
        m_attributes->resize (i + 1);
    shared_ptr<Attribute> p(attribute);
    (*m_attributes)[i] = p;
}

void Element::StoreAttributes (
    vector<NameSemanticValue*>& list, AttributesInfo& infos)
{
    for_each (list.begin (), list.end (), storeAttribute(*this, infos));
}

ostream& Element::PrintAttributes (ostream& ostr) const
{
    ostream_iterator< boost::shared_ptr<Attribute> > oi (ostr, " ");
    if (m_attributes != 0)
	copy (m_attributes->begin (), m_attributes->end (), oi);
    return ostr;
}

