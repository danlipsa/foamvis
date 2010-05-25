/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Base class for Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "Enums.h"
class Attribute;
class AttributesInfo;
class NameSemanticValue;

/**
 * Base class for  Vertex, Edge, Face and Body.  Encapsulates a vector
 * of attributes.
 */
class Element
{
public:
    /**
     * Constructor for the Element
     */
    Element(size_t id,
	    ElementStatus::Name status) : 
	m_attributes(0),
	m_id (id), 
	m_status (status)
    {
    }
    Element (const Element& other);

    /**
     * Sets an attribute for the element
     * @param i position in the vector of attributes
     * @param attribute pointer to the attribute to set
     */
    void SetAttribute (size_t i, Attribute* attribute);
    /**
     * Stores all attributes in the element
     * @param list the list of attributes values
     * @param info information about each attribute and how it is created
     */
    void StoreAttributes (
        vector<NameSemanticValue*>& list, AttributesInfo& info);
    /**
     * Gets the original index of this element
     */
    size_t GetId () const
    {
	return m_id;
    }    
    ElementStatus::Name GetStatus () const
    {
	return m_status;
    }
    void SetStatus (ElementStatus::Name status) 
    {
	m_status = status;
    }

protected:
    typedef vector< boost::shared_ptr<Attribute> > Attributes;

protected:
    /**
     * Pretty print attributes of an element
     * @param ostr where to print
     * @param info information about attributes
     */
    ostream& printAttributes (
	ostream& ostr, const AttributesInfo& info) const;

protected:
    /**
     * Vector of attributes
     */
    boost::scoped_ptr<Attributes> m_attributes;
    /**
     * The original index for this element
     */
    size_t m_id;
    ElementStatus::Name m_status;

protected:
    const static size_t INVALID_INDEX;
};

#endif //__ELEMENT_H__

// Local Variables:
// mode: c++
// End:
