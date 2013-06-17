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
            if (index == INVALID_INDEX)
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


// Methods
// ======================================================================

Element::Element(size_t id, ElementStatus::Enum duplicateStatus) : 
    m_id (id), 
    m_duplicateStatus (duplicateStatus)
{
}

Element::Element (const Element& other) :
    m_id (other.m_id),
    m_duplicateStatus (other.m_duplicateStatus)
{
    m_attributes.resize (other.m_attributes.size ());
    m_attributes = other.m_attributes;
}


void Element::SetAttribute (size_t i, Attribute* attribute)
{
    resizeAllowIndex (&m_attributes, i);
    boost::shared_ptr<Attribute> p(attribute);
    m_attributes[i] = p;
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

ostream& Element::PrintAttributes (ostream& ostr,
				   const AttributesInfo* infos) const
{
    if (HasAttributes ())
	for (size_t i = 0; i < m_attributes.size (); ++i)
	    if (HasAttribute (i))
	    {
		if (infos != 0)
		    ostr << infos->GetAttributeName (i) << ": ";
		ostr << m_attributes[i] << " ";
	    }
    return ostr;
}

string Element::GetStringId () const
{
    ostringstream ostr;
    ostr << setw(4) << GetId ();
    return ostr.str ();
}


template<typename T, typename TValue>
TValue Element::GetAttribute (size_t i) const
{
    RuntimeAssert (HasAttribute (i),
		   "Attribute does not exist at index ", i, 
		   " for element ", GetId ());
    return *boost::static_pointer_cast<T> (m_attributes[i]);
}

template<typename T, typename TValue>
void Element::SetAttribute (size_t i, TValue value)
{
    resizeAllowIndex (&m_attributes, i);
    boost::shared_ptr<T> attribute = 
	boost::static_pointer_cast<T> (m_attributes[i]);
    if (attribute == 0)
    {
	attribute = boost::shared_ptr<T> (new T (value));
	m_attributes[i] = attribute;
    }
    else
	attribute->set (value);
}

bool Element::HasAttribute (size_t i) const
{
    return i < m_attributes.size () &&	m_attributes[i] != 0;
}

bool Element::HasAttributes () const
{
    return ! m_attributes.empty ();
}

// Template instantiations
// ======================================================================

/** @cond */

template void Element::SetAttribute<RealAttribute, double>(unsigned long, double);
template void Element::SetAttribute<ColorAttribute, Color::Enum>(unsigned long, Color::Enum);
template void Element::SetAttribute<IntegerArrayAttribute, std::vector<int, std::allocator<int> > const> (unsigned long, std::vector<int, std::allocator<int> > const);
template double Element::GetAttribute<RealAttribute, double>(unsigned long) const;
template Color::Enum Element::GetAttribute<ColorAttribute, Color::Enum>(unsigned long) const;
template int Element::GetAttribute<IntegerAttribute, int>(unsigned long) const;
template std::vector<int, std::allocator<int> > const Element::GetAttribute<IntegerArrayAttribute, std::vector<int, std::allocator<int> > const>(unsigned long) const;

/** @endcond */
