#include <algorithm>
#include "Element.h"
#include "ElementUtils.h"
#include "Attribute.h"
#include "AttributeInfo.h"
#include "NameSemanticValue.h"
#include "SemanticError.h"
using namespace std;

class storeAttribute : 
    public unary_function <const NameSemanticValue*, void>
{
public:
    storeAttribute(
        Element& where, AttributesInfo& infos) : 
        m_where (where), m_infos (infos) {}
    void operator() (const NameSemanticValue* nameSemanticValue)
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
                    index, creator(nameSemanticValue->GetSemanticValue (), 
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
    Element& m_where;
    AttributesInfo& m_infos;
};

class printAttribute : 
    public unary_function <const Attribute*, void>
{
public:
    printAttribute(
        ostream& ostr, AttributesInfo& infos) : 
        m_ostr (ostr), m_infos (infos), m_index(0) {}
    void operator() (const Attribute* attribute)
    {
        const char* name = m_infos.GetAttributeName (m_index++);
        m_ostr << name << " " << *attribute << " ";
    }
private:
    ostream& m_ostr;
    AttributesInfo& m_infos;
    unsigned int m_index;
};


Element::~Element()
{
        if (m_attributes != 0)
        {
                for_each(m_attributes->begin (), m_attributes->end (), 
                        DeleteElementPtr<Attribute>);
                delete m_attributes;
        }
}

void Element::SetAttribute (unsigned int i, const Attribute* attribute)
{
        if (m_attributes == 0)
                m_attributes = new vector<const Attribute*> ();
    if (i >= m_attributes->size ())
        m_attributes->resize (i + 1);
    (*m_attributes)[i] = attribute;
}

void Element::StoreAttributes (
    vector<NameSemanticValue*>& list, AttributesInfo& infos)
{
    for_each (list.begin (), list.end (), storeAttribute(*this, infos));
}

ostream& Element::PrintAttributes(ostream& ostr, AttributesInfo& infos)
{
        if (m_attributes != 0)
                for_each (m_attributes->begin (), m_attributes->end (),
                        printAttribute (ostr, infos));
    return ostr;
}
