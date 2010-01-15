/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Base class for Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "Point.h"

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
    Element() : m_attributes(0), m_averageCalculated(false)
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
    void SetAttribute (unsigned int i, const Attribute* attribute);
    /**
     * Stores all attributes in the element
     * @param list the list of attributes values
     * @param info information about each attribute and how it is created
     */
    void StoreAttributes (
        std::vector<NameSemanticValue*>& list, AttributesInfo& info);
    /**
     * Pretty print attributes of an element
     * @param ostr where to print
     * @param info information about attributes
     */
    std::ostream& PrintAttributes (
	std::ostream& ostr, AttributesInfo& info) const;
    /**
     * Calculate the average point of this element
     */
    virtual void CalculateAverage () = 0;
    /**
     * Gets the average of this element. It has to be calculated first
     */
    const Point& GetAverage ();
protected:
    /**
     * Vector of attributes
     */
    std::vector<const Attribute*>* m_attributes;
    /**
     * Average of vertices
     */
    Point m_average;
    /**
     * Was the average already calculated for this edge
     */
    bool m_averageCalculated;
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};

#endif //__ELEMENT_H__

// Local Variables:
// mode: c++
// End:
