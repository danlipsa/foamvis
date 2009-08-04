#ifndef __ATTRIBUTE_INFO_H__
#define __ATTRIBUTE_INFO_H__

#include <limits.h>
#include <map>
#include <set>
#include "AttributeCreator.h"
#include "ElementUtils.h"

using namespace std;


class AttributeInfo
{
public:
    AttributeInfo (unsigned int index, AttributeCreator* creator) : 
    m_index (index), m_creator (creator) {}
    unsigned int GetIndex () {return m_index;}
    AttributeCreator* GetCreator () {return m_creator;}
	
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
    const unsigned int INVALID_INDEX;
private:
    set<const char*, LessThanNoCase> m_loadAttribute;
    map<const char*, AttributeInfo*, LessThanNoCase> m_nameInfo;
    unsigned int m_currentIndex;
    bool m_loadAll;
};

#endif
