/**
 * @file   AttributeInfo.h
 * @author Dan R. Lipsa
 * Information about attributes for vertices, edges, ... such as
 * their creators and their index in the array of attributes carried by each
 * element.
 */
#ifndef __ATTRIBUTE_INFO_H__
#define __ATTRIBUTE_INFO_H__

#include "Comparisons.h"

class AttributeCreator;

/**
 * Information about an attribute attached to a vertex, edge, ...
 */
class AttributeInfo
{
public:
    /**
     * Constructor
     *
     * @param index index in the  array of attributes attached to each
     * element where this attribute is stored.
     * @param creator functor that knows how to create this attribute.
     */
    AttributeInfo (size_t index, boost::shared_ptr<AttributeCreator> creator);
    /**
     * Gets the index where this attribute is stored
     * @return  index in  the  array of  attributes  attached to  each
     * element where this attribute is stored.
     */
    size_t GetIndex () {return m_index;}
    /**
     * Gets the functor that knows how to create this attribute
     * @return functor that knows how to create this attribute
     */
    AttributeCreator& GetCreator ()
    {
	return *m_creator;
    }
    string ToString () const;

    /**
     * Constant used to signal an attribute that will not be stored.
     */
    static const size_t INVALID_INDEX; 
private:
    /**
     * The index where this attribute is going to be stored
     */
    size_t m_index;
    /**
     * Knows how to create a kind of attribute.
     */
    boost::shared_ptr<AttributeCreator> m_creator;
};

inline ostream& operator<< (ostream& ostr, const AttributeInfo& ai)
{
    return ostr << ai.ToString ();
}


/**
 * Information about all attributes
 */
class AttributesInfo
{
public:
    typedef map<string, 
		boost::shared_ptr<AttributeInfo>, LessThanNoCase> NameInfoMap;

public:
    /**
     * Constructor
     */
    AttributesInfo ();
    /**
     * Stores information about an attribute. The attributte is loaded
     * only if m_loadAll is set.
     * @param name name of the attribute
     * @param creator functor that creates the attribute
     * @return the index where the attribute is stored or INVALID_INDEX if
     *         the attribute is not stored
     */
    size_t AddAttributeInfo (
	const char* name, boost::shared_ptr<AttributeCreator> creator);
    /**
     * The same as AddAttributeInfo but always load the attribute from the
     * datafile
     */
    size_t AddAttributeInfoLoad (
	const char* name, boost::shared_ptr<AttributeCreator> creator);
    /**
     * Gets information about a certain attribute
     * @param name the name of the attribute we are interested in
     * @return information about the attribute
     */
    boost::shared_ptr<AttributeInfo> GetAttributeInfo (const char* name) const;
    /**
     * Gets the name of an attribute stored at a certain index in the array of
     * attributes attached to a certain element (vertex, edge, ...)
     * @param index where the attribute is stored
     * @return name of the attribute
     */
    const char* GetAttributeName (size_t index) const;
    string ToString () const;

private:
    /**
     * All the attributes values that should be loaded from the data file
     */
    set<string, LessThanNoCase> m_loadAttribute;
    /**
     * Map between an attribute name and an attribute info
     */
    NameInfoMap m_nameInfo;
    /**
     * Index where a  newly added attribute will be  store in an array
     * of attributes attached to an element.
     */
    size_t m_currentIndex;
    /**
     * Specify if we should load all attributes from the DMP file or not.
     */
    bool m_loadAll;
};

inline ostream& operator<< (ostream& ostr, const AttributesInfo& ai)
{
    return ostr << ai.ToString ();
}



#endif //__ATTRIBUTE_INFO_H__

// Local Variables:
// mode: c++
// End:
