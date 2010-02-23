/**
 * @file Element.cpp
 * @author Dan R. Lipsa
 * 
 * Definitions for methods of the Element class.
 */
#include "Element.h"
#include "ElementUtils.h"
#include "AttributeInfo.h"
#include "SemanticError.h"


/**
 * Functor that stores an attribute in an element.
 */
class storeAttribute : 
    public unary_function < NameSemanticValue*, void>
{
public:
    /**
     * Constructor for the functor
     * @param where element where to store the attribute
     * @param infos information about attributes and how are they created
     */
    storeAttribute(
        Element& where, AttributesInfo& infos) : 
        m_where (where), m_infos (infos) {}
    /**
     * Stores an attribute
     * @param nameSemanticValue Name and value of the attribute
     */
    void operator() (NameSemanticValue* nameSemanticValue)
    {
        try
        {
            AttributeInfo* info = 
                m_infos.GetAttributeInfo (nameSemanticValue->GetName ());
            if (info == 0)
                throw SemanticError (string("Attribute \"") + 
                                     nameSemanticValue->GetName () + 
                                     "\" was not defined");
            unsigned int index = info->GetIndex ();
            if (index == AttributeInfo::INVALID_INDEX)
                return;
            else
            {
                AttributeCreator& creator = *(info->GetCreator ());
                m_where.SetAttribute (
                    index, 
		    creator(nameSemanticValue->GetSemanticValue (), 
			    nameSemanticValue->GetType ()));
            }
        }
        catch (SemanticError& e)
        {
            throw SemanticError (string(nameSemanticValue->GetName ()) + ": "
                                 + e.what ());
        }
    }
private:
    /**
     * Element where the attribute is stored
     */
    Element& m_where;
    /**
     * Information about all attributes and how they are created.
     */
    AttributesInfo& m_infos;
};

/**
 * Functor that prints an attribute
 */
class printAttribute : 
    public unary_function < Attribute*, void>
{
public:
    /**
     * Constructor for the functor
     * @param ostr where to print the attribute
     * @param infos information about attributes
     */
    printAttribute(
        ostream& ostr, AttributesInfo& infos) : 
        m_ostr (ostr), m_infos (infos), m_index(0) {}
    /**
     * Functor that prints an attribute
     * @param attribute to be printed.
     */
    void operator() (boost::shared_ptr<Attribute> attribute)
    {
	const char* name = m_infos.GetAttributeName (m_index++);
        m_ostr << name << " " << *attribute << " ";
    }
private:
    /**
     * Stream where to print
     */
    ostream& m_ostr;
    /**
     * Information about attributes
     */
    AttributesInfo& m_infos;
    /**
     * Current attribute index
     */
    unsigned int m_index;
};

Element::~Element()
{
    delete m_attributes;
}

void Element::SetAttribute (unsigned int i, Attribute* attribute)
{
    using namespace boost;
    if (m_attributes == 0)
	m_attributes = new vector<shared_ptr<Attribute> > ();
    if (i >= m_attributes->size ())
        m_attributes->resize (i + 1);
    shared_ptr<Attribute> p(attribute);
    (*m_attributes)[i] = p;
}

void Element::StoreAttributes (
    vector<NameSemanticValue*>& list, AttributesInfo& infos)
{
    for_each (list.begin (), list.end (), storeAttribute(*this, infos));
}

ostream& Element::PrintAttributes (ostream& ostr, AttributesInfo& infos) 
{
    if (m_attributes != 0)
	for_each (m_attributes->begin (), m_attributes->end (),
		  printAttribute (ostr, infos));
    return ostr;
}

