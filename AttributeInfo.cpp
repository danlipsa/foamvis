#include <algorithm>
#include "AttributeInfo.h"

class indexEqual : unary_function<
    pair<const char*, const AttributeInfo*>, bool >
{
public:
    indexEqual (unsigned int index) : m_index (index) {}
    bool operator() (pair<const char*, AttributeInfo*> p)
    {
	if (p.second->GetIndex () == m_index)
	    return true;
	else
	    return false;
    }
private:
    unsigned int m_index;
};


AttributesInfo::AttributesInfo () : 
    INVALID_INDEX(UINT_MAX), m_loadAll (true) 
{}

void AttributesInfo::AddAttributeInfo (
    const char* name, AttributeCreator* creator)
{
    if (m_loadAll || 
	m_loadAttribute.find (name) != m_loadAttribute.end ())
	m_nameInfo[name] = new AttributeInfo (m_currentIndex++, creator);
    else
    {
	m_nameInfo[name] = new AttributeInfo (INVALID_INDEX, 0);
	delete creator;
    }
}

const char* AttributesInfo::GetAttributeName (unsigned int index)
{
    map<const char*, AttributeInfo*>::iterator it = 
	find_if (m_nameInfo.begin (), m_nameInfo.end (), indexEqual (index));
    return it->first;
}

AttributeInfo* AttributesInfo::GetAttributeInfo (const char* name) const
{
    map<const char*, AttributeInfo*>::const_iterator it = 
	m_nameInfo.find (name);
    if (it == m_nameInfo.end ())
	return 0;
    else
	return it->second;
}
