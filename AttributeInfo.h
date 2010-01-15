/**
 * @file   AttributeInfo.h
 * @author Dan R. Lipsa
 * Information about attributes for vertices, edges, ... such as
 * their creators and their index in the array of attributes carried by each
 * element.
 */
#ifndef __ATTRIBUTE_INFO_H__
#define __ATTRIBUTE_INFO_H__

#include "AttributeCreator.h"
#include "ElementUtils.h"

class AttributeInfo
{
public:
    AttributeInfo (unsigned int index, AttributeCreator* creator) : 
    m_index (index), m_creator (creator) {}
    unsigned int GetIndex () {return m_index;}
    AttributeCreator* GetCreator () {return m_creator;}
    const static unsigned int INVALID_INDEX = UINT_MAX; 
private:
    unsigned int m_index;
    AttributeCreator* m_creator;
};

class AttributesInfo
{
public:
    AttributesInfo ();
    void AddAttributeInfo (const char* name, AttributeCreator* creator);
    void Load (const char* name) {m_loadAttribute.insert (name);}
    void LoadAll () {m_loadAll = true;}
    AttributeInfo* GetAttributeInfo (const char* name) const;
    const char* GetAttributeName (unsigned int index);
private:
    std::set<const char*, LessThanNoCase> m_loadAttribute;
    std::map<const char*, AttributeInfo*, LessThanNoCase> m_nameInfo;
    unsigned int m_currentIndex;
    bool m_loadAll;
};

#endif //__ATTRIBUTE_INFO_H__

// Local Variables:
// mode: c++
// End:
