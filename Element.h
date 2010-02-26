/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Base class for Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

class Attribute;
class AttributesInfo;
class NameSemanticValue;
class Data;

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
    Element(unsigned int originalIndex, Data* data,
	    bool duplicate) : 
	m_attributes(0),
	m_originalIndex (originalIndex), 
	m_data (data),
	m_duplicate (duplicate)
    {}
    /**
     * Destructor for the Element
     */
    ~Element();
    /**
     * Sets an attribute for the element
     * @param i position in the vector of attributes
     * @param attribute pointer to the attribute to set
     */
    void SetAttribute (unsigned int i, Attribute* attribute);
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
    unsigned int GetOriginalIndex ()  {return m_originalIndex;}
    /**
     * Pretty print attributes of an element
     * @param ostr where to print
     * @param info information about attributes
     */
    ostream& PrintAttributes (
	ostream& ostr, AttributesInfo& info) ;

    bool IsDuplicate ()  {return m_duplicate;}
    void SetDuplicate (bool duplicate) {m_duplicate = duplicate;}
protected:
    /**
     * Vector of attributes
     */
    vector<boost::shared_ptr<Attribute> >* m_attributes;
    /**
     * The original index for this element
     */
    unsigned int m_originalIndex;
    const static unsigned int INVALID_INDEX;
    Data* m_data;
    bool m_duplicate;
};

#endif //__ELEMENT_H__

// Local Variables:
// mode: c++
// End:
