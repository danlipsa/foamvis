/**
 * @file   AttributeInfo.cpp
 * @author Dan R. Lipsa
 *
 * Class member definitions for classes that contain information about
 * attributes  for vertices,  edges, ...  such as  their  creators and
 * their index in the array of attributes carried by each element.
 */
#include "AttributeInfo.h"

/**
 * Functor that checks if (name, AttributeInfo*) pair contains a given index
 */
class indexEqual : unary_function<
    pair<const char*, const AttributeInfo*>, bool >
{
public:
    /**
     * Constructor
     * @param index the index value that will be checked.
     */
    indexEqual (unsigned int index) : m_index (index) {}
    /**
     * Functor that checks if a (name, AttributeInfo*) pair contains a
     * given index
     */
    bool operator() (pair<const char*, AttributeInfo*> p)
    {
        if (p.second->GetIndex () == m_index)
            return true;
        else
            return false;
    }
private:
    /**
     * Index  value   that  is   checked  against  values   in  (name,
     * AttributeInfo*) pairs
     */
    unsigned int m_index;
};

AttributesInfo::AttributesInfo () : 
m_currentIndex(0), m_loadAll (false)
{}

void AttributesInfo::AddAttributeInfo (
    const char* name, AttributeCreator* creator)
{
    if (m_loadAll || 
        m_loadAttribute.find (name) != m_loadAttribute.end ())
        m_nameInfo[name] = new AttributeInfo (m_currentIndex++, creator);
    else
    {
        m_nameInfo[name] = new AttributeInfo (AttributeInfo::INVALID_INDEX, 0);
        delete creator;
    }
}

const char* AttributesInfo::GetAttributeName (unsigned int index)
{
    map<const char*, AttributeInfo*, LessThanNoCase>::iterator it = 
        find_if (m_nameInfo.begin (), m_nameInfo.end (), indexEqual (index));
    return it->first;
}

AttributeInfo* AttributesInfo::GetAttributeInfo (const char* name) const
{
    map<const char*, AttributeInfo*, LessThanNoCase>::const_iterator it = 
        m_nameInfo.find (name);
    if (it == m_nameInfo.end ())
        return 0;
    else
        return it->second;
}
