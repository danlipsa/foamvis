#include "AttributeInfo.h"

void AttributesInfo::AddAttributeInfo (
    const char* name, AttributeCreator* creator)
{
    set<const char*>::iterator it = m_attributeStored.find (name);
    if (it == m_attributeStored.end ())
    {
	m_attributes[name] = new AttributeInfo (INVALID_INDEX, 0);
	delete creator;
    }
    else
	m_attributes[name] = new AttributeInfo (m_currentIndex++, creator);
}
