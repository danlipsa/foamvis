/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Base class for Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <iostream>
#include <vector>

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
        Element() : m_attributes(0) {}
    ~Element();
    void SetAttribute (unsigned int i, const Attribute* attribute);
    void StoreAttributes (
        std::vector<NameSemanticValue*>& list, AttributesInfo& info);
    std::ostream& PrintAttributes (std::ostream& ostr, AttributesInfo& infos);
protected:
    std::vector<const Attribute*>* m_attributes;
};

#endif //__ELEMENT_H__

// Local Variables:
// mode: c++
// End:
