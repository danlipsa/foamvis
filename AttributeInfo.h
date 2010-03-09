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
    AttributeInfo (unsigned int index, AttributeCreator* creator) : 
	m_index (index), m_creator (creator) {}
    /**
     * Gets the index where this attribute is stored
     * @return  index in  the  array of  attributes  attached to  each
     * element where this attribute is stored.
     */
    unsigned int GetIndex () {return m_index;}
    /**
     * Gets the functor that knows how to create this attribute
     * @return functor that knows how to create this attribute
     */
    AttributeCreator* GetCreator () {return m_creator;}
    /**
     * Constant used to signal an attribute that will not be stored.
     */
     static const unsigned int INVALID_INDEX = UINT_MAX; 
private:
    /**
     * The index where this attribute is going to be stored
     */
    unsigned int m_index;
    /**
     * Knows how to create a kind of attribute.
     */
    AttributeCreator* m_creator;
};

/**
 * Information about all attributes
 */
class AttributesInfo
{
public:
    /**
     * Constructor
     */
    AttributesInfo ();
    /**
     * Stores information about an attribute
     * @param name name of the attribute
     * @param creator functor that creates the attribute
     */
    void AddAttributeInfo (const char* name, AttributeCreator* creator);
    /**
     * Specify that a certain attribute should be loaded from the data file
     * @param name the attribute that should be loaded from the data file
     */
    void Load (const char* name) {m_loadAttribute.insert (name);}
    /**
     * All attributes should be loaded from the data file
     */
    void LoadAll () {m_loadAll = true;}
    /**
     * Gets information about a certain attribute
     * @param name the name of the attribute we are interested in
     * @return information about the attribute
     */
    AttributeInfo* GetAttributeInfo (const char* name) ;
    /**
     * Gets the name of an attribute stored at a certain index in the array of
     * attributes attached to a certain element (vertex, edge, ...)
     * @param index where the attribute is stored
     * @return name of the attribute
     */
     const char* GetAttributeName (unsigned int index) const;
private:
    /**
     * All the attributes values that should be loaded from the data file
     */
    set<const char*, LessThanNoCase> m_loadAttribute;
    /**
     * Map between an attribute name and an attribute info
     */
    map<const char*, AttributeInfo*, LessThanNoCase> m_nameInfo;
    /**
     * Index where a  newly added attribute will be  store in an array
     * of attributes attached to an element.
     */
    unsigned int m_currentIndex;
    /**
     * Specify if we should load all attributes from the DMP file or not.
     */
    bool m_loadAll;
};

#endif //__ATTRIBUTE_INFO_H__

// Local Variables:
// mode: c++
// End:
