#ifndef __ATTRIBUTE_INFO_H__
#define __ATTRIBUTE_INFO_H__

#include <limits.h>
#include <map>
#include <set>
#include "AttributeCreator.h"

using namespace std;

class AttributeInfo
{
public:
    AttributeInfo (int index, AttributeCreator* creator) : 
	m_index (index), m_creator (creator) {}
private:
    int m_index;
    AttributeCreator* m_creator;
};

class AttributesInfo
{
public:
    AttributesInfo () : INVALID_INDEX(UINT_MAX) {}
    void AddAttributeInfo (const char* name, AttributeCreator* creator);
    void Store (const char* name) {m_attributeStored.insert (name);}

    const unsigned int INVALID_INDEX;
private:
    set<const char*> m_attributeStored;
    map<const char*, AttributeInfo*> m_attributes;
    unsigned int m_currentIndex;
};

#endif
