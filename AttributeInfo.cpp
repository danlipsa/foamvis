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
#include "Debug.h"
#include "Utils.h"


// AttributeInfo Methods
// ======================================================================

AttributeInfo::AttributeInfo (
    size_t index, boost::shared_ptr<AttributeCreator> creator) :
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
    const char* name, boost::shared_ptr<AttributeCreator> creator)
{
    if (m_loadAll || 
        m_loadAttribute.find (name) != m_loadAttribute.end ())
    {
        m_nameInfo[name] = boost::make_shared<AttributeInfo> (
	    m_currentIndex, creator);
	return m_currentIndex++;
    }
    else
    {
	boost::shared_ptr<AttributeCreator> nullPtr;
        m_nameInfo[name] = boost::make_shared<AttributeInfo> (
	    INVALID_INDEX, nullPtr);
	return INVALID_INDEX;
    }
}

size_t AttributesInfo::AddAttributeInfoLoad (
    const char* name, boost::shared_ptr<AttributeCreator> creator)
{
    m_loadAttribute.insert (name);
    return AddAttributeInfo (name, creator);
}


const char* AttributesInfo::GetAttributeName (size_t index) const
{
    NameInfoMap::const_iterator it = 
        find_if (
	    m_nameInfo.begin (), m_nameInfo.end (), 
	    boost::bind (&AttributeInfo::GetIndex, 
			 boost::bind (&NameInfoMap::value_type::second, _1)) 
	    == index);
    return it->first.c_str ();
}

boost::shared_ptr<AttributeInfo> AttributesInfo::GetAttributeInfo (
    const char* name) const
{
    NameInfoMap::const_iterator it = m_nameInfo.find (name);
    RuntimeAssert (it != m_nameInfo.end (), 
		   "Attribute \"", name, "\" was not defined");
    return it->second;
}

string AttributesInfo::ToString () const
{
    ostringstream ostr;
    ostream_iterator<string> out(ostr, " ");
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
