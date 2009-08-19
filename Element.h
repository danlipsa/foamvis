/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <iostream>
#include <vector>
using namespace std;

class Attribute;
class AttributesInfo;

class NameSemanticValue;
class Element
{
public:
        Element() : m_attributes(0) {}
    ~Element();
    void SetAttribute (unsigned int i, const Attribute* attribute);
    void StoreAttributes (
        vector<NameSemanticValue*>& list, AttributesInfo& info);
    ostream& PrintAttributes (ostream& ostr, AttributesInfo& infos);
protected:
    vector<const Attribute*>* m_attributes;
};


#endif
// Local Variables:
// mode: c++
// End:
