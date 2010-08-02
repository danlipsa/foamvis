/**
 * @file   AttributeInfo.cpp
 * @author Dan R. Lipsa
 *
 * Class member definitions for classes that contain information about
 * attributes  for vertices,  edges, ...  such as  their  creators and
 * their index in the array of attributes carried by each element.
 */
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Comparisons.h"

/**
 * Functor that checks if (name, AttributeInfo*) pair contains a given index
 */
class indexEqual : unary_function<
    pair<const char*,  AttributeInfo*>, bool >
{
public:
    /**
     * Constructor
     * @param index the index value that will be checked.
     */
    indexEqual (size_t index) : m_index (index) {}
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
    size_t m_index;
};


// AttributeInfo Methods
// ======================================================================

AttributeInfo::AttributeInfo (
    size_t index, auto_ptr<AttributeCreator>& creator) :
    m_index (index),
    m_creator (creator)
{
}

string AttributeInfo::ToString () const
{
    ostringstream ostr;
    ostr << "index: " << m_index;
    return ostr.str ();
}


// AttributesInfo Methods
// ======================================================================


AttributesInfo::AttributesInfo () : 
m_currentIndex(0), m_loadAll (false)
{}

size_t AttributesInfo::AddAttributeInfo (
    const char* name, auto_ptr<AttributeCreator> creator)
{
    if (m_loadAll || 
        m_loadAttribute.find (name) != m_loadAttribute.end ())
    {
        m_nameInfo[name] = new AttributeInfo (m_currentIndex, creator);
	return m_currentIndex++;
    }
    else
    {
	auto_ptr<AttributeCreator> nullPtr;
        m_nameInfo[name] = new AttributeInfo (
	    AttributeInfo::INVALID_INDEX, nullPtr);
	return AttributeInfo::INVALID_INDEX;
    }
}

size_t AttributesInfo::AddAttributeInfoLoad (
    const char* name, auto_ptr<AttributeCreator> creator)
{
    m_loadAttribute.insert (name);
    return AddAttributeInfo (name, creator);
}



const char* AttributesInfo::GetAttributeName (size_t index) const
{
    map<const char*, AttributeInfo*, LessThanNoCase>::const_iterator it = 
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

string AttributesInfo::ToString () const
{
    ostringstream ostr;
    ostream_iterator<const char*> out(ostr, " ");
    ostr << "Load attributes: ";
    copy (m_loadAttribute.begin (), m_loadAttribute.end (), out);
    ostr << endl;
    
    ostr << "Attributes info: ";
    BOOST_FOREACH (NameInfoMap::value_type p, m_nameInfo)
    {
	ostr << p.first << ": " << *p.second << endl;
    }
    
    ostr << "Next index: " << m_currentIndex << endl;
    ostr << "Load all: " << m_loadAll << endl;
    return ostr.str ();
}
