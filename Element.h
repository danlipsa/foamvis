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
    Element(size_t id, ElementStatus::Enum duplicateStatus);
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
        vector<NameSemanticValue*>& list, const AttributesInfo& info);
    void StoreAttribute (
        const char* name, double r, const AttributesInfo& info);

    /**
     * Gets the original index of this element. Note that this might
     * not be unique for vertices, edges and faces as a result of
     * duplication because of periodic boundary conditions. For bodies
     * it is unique for a given foam.
     */
    size_t GetId () const
    {
	return m_id;
    }    
    
    string GetStringId () const;

    ElementStatus::Enum GetDuplicateStatus () const
    {
	return m_duplicateStatus;
    }
    void SetDuplicateStatus (ElementStatus::Enum duplicateStatus) 
    {
	m_duplicateStatus = duplicateStatus;
    }
    /**
     * Pretty print attributes of an element
     * @param ostr where to print
     * @param info information about attributes
     */
    ostream& PrintAttributes (ostream& ostr) const;
    double GetRealAttribute (size_t i) const;
    const vector<int>& GetIntegerArrayAttribute (size_t i) const;
    void SetRealAttribute (size_t i, double value);
    bool HasAttribute (size_t i) const;
    bool HasAttributes () const;
private:
    void storeAttribute (
	const NameSemanticValue& nv, const AttributesInfo& infos);

protected:
    typedef vector< boost::shared_ptr<Attribute> > Attributes;

protected:
    /**
     * Vector of attributes
     */
    boost::scoped_ptr<Attributes> m_attributes;
    /**
     * The original index for this element
     */
    size_t m_id;
    ElementStatus::Enum m_duplicateStatus;
};

#endif //__ELEMENT_H__

// Local Variables:
// mode: c++
// End:
