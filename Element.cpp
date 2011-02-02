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
#include "Utils.h"
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
        Element& where, const AttributesInfo& infos) : 
        m_where (where), m_infos (infos) {}
    /**
     * Stores an attribute
     * @param nameSemanticValue Name and value of the attribute
     */
    void operator() (const NameSemanticValue* nv)
    {
	operator () (*nv);
    }

    void operator() (const NameSemanticValue& nv)
    {
        try
        {
	    boost::shared_ptr<AttributeInfo> info = 
                m_infos.GetAttributeInfo (nv.GetName ());
            size_t index = info->GetIndex ();
            if (index == AttributeInfo::INVALID_INDEX)
                return;
            else
            {
                AttributeCreator& creator = info->GetCreator ();
                m_where.SetAttribute (
                    index, creator(nv.GetSemanticValue (), nv.GetType ()));
            }
        }
        catch (exception& e)
        {
            throw logic_error (string(nv.GetName ()) + ": "
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
    const AttributesInfo& m_infos;
};


// Static Fields
// ======================================================================
const size_t Element::INVALID_INDEX = numeric_limits<size_t>::max ();

// Methods
// ======================================================================

Element::Element(size_t id, ElementStatus::Enum duplicateStatus) : 
    m_attributes(0),
    m_id (id), 
    m_duplicateStatus (duplicateStatus)
{
}

Element::Element (const Element& other) :
    m_id (other.m_id),
    m_duplicateStatus (other.m_duplicateStatus)
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

void Element::storeAttribute (
    const NameSemanticValue& nv, const AttributesInfo& infos)
{
    ::storeAttribute (*this, infos) (nv);
}

void Element::StoreAttribute (
    const char* name, double r, const AttributesInfo& infos)
{
    NameSemanticValue nv (name, r);
    storeAttribute (nv, infos);
}



void Element::StoreAttributes (
    vector<NameSemanticValue*>& list, const AttributesInfo& infos)
{
    for_each (list.begin (), list.end (), ::storeAttribute(*this, infos));
}

ostream& Element::PrintAttributes (ostream& ostr) const
{
    ostream_iterator< boost::shared_ptr<Attribute> > oi (ostr, " ");
    if (m_attributes != 0)
	copy (m_attributes->begin (), m_attributes->end (), oi);
    return ostr;
}

string Element::GetStringId () const
{
    ostringstream ostr;
    ostr << setw(4) << GetId ();
    return ostr.str ();
}

double Element::GetRealAttribute (size_t i) const
{
    RuntimeAssert (m_attributes != 0 && i < m_attributes->size (),
		   "Attribute does not exist at index ", i, 
		   " for element ", GetId ());
    return *boost::static_pointer_cast<RealAttribute> ((*m_attributes)[i]);
}

void Element::SetRealAttribute (size_t i, double value)
{
    RuntimeAssert (m_attributes != 0 && i < m_attributes->size (),
		   "Attribute does not exist at index ", i, 
		   " for element ", GetId ());
    RealAttribute& attribute = *boost::static_pointer_cast<RealAttribute> (
	(*m_attributes)[i]);
    attribute.set (value);
}

bool Element::ExistsAttribute (size_t i) const
{
    return m_attributes != 0 && i < m_attributes->size () &&
	(*m_attributes)[i] != 0;
}
