/**
 * @file   NameSemanticValue.h
 * @author Dan R. Lipsa
 * @brief Tupple (name, type, value) used for a vertex, edge, face, and
 * body attribute.
 * @ingroup parser
 */
#ifndef __NAME_SEMANTIC_VALUE_H__
#define __NAME_SEMANTIC_VALUE_H__

#include "Enums.h"
#include "EvolverData_yacc.h"


/**
 * @brief Tupple (name, type, value) used for a vertex, edge, face, and
 * body attribute.
 */
class NameSemanticValue
{
public:
    /**
     * Constructor for an attribute with type INT
     * @param name name of the attribute
     * @param i the value of the attribute
     */
    NameSemanticValue (const char* name, int i) : m_name(name)
    {
        m_semanticValue.m_int = i;
        m_type = AttributeType::INT;
    }
    /**
     * Constructor for an attribute with type COLOR
     * @param name name of the attribute
     * @param color color of the attribute
     */
    NameSemanticValue (const char* name, Color::Enum color) : m_name(name)
    {
        m_semanticValue.m_color = color;
        m_type = AttributeType::COLOR;
    }
    /**
     * Constructor for an attribute with type REAL
     * @param name name of the attribute
     * @param r value of the attribute
     */
    NameSemanticValue (const char* name, double r) : m_name(name)
    {
        m_semanticValue.m_real = r;
        m_type = AttributeType::REAL;
    }
    /**
     * Constructor for an attribute with type INT_ARRAY
     * @param name name of the attribute
     * @param intList value of the attribute
     */
    NameSemanticValue (const char* name, vector<int>* intList) : 
        m_name(name)
    {
        m_semanticValue.m_intList = intList;
        m_type = AttributeType::INT_ARRAY;
    }
    /**
     * Constructor for an attribute with type REAL_ARRAY
     * @param name name of the attribute
     * @param realList value of the attribute
     */
    NameSemanticValue (const char* name, vector<double>* realList) : 
        m_name(name)
    {
        m_semanticValue.m_realList = realList;
        m_type = AttributeType::REAL_ARRAY;
    }
    /**
     * Gets the name of the attribute
     * @return the name of the attribute
     */
    const char* GetName () const
    {
        return m_name;
    }
    /**
     * Gets the value of the attribute (as a semantic_type union)
     * @return the value of the attribute as a semantic_type union
     */
    const EvolverData::parser::semantic_type& GetSemanticValue () const
    {
        return m_semanticValue;
    }
    /**
     * Gets the type of the attribute
     * @return the type of the attribute
     */
    AttributeType::Enum GetType () const
    {
        return m_type;
    }

public:
    /**
     * Adds this attribute to a list of attributes
     */
    static vector<NameSemanticValue*>* PushBack (
	vector<NameSemanticValue*>* listSoFar, 
	NameSemanticValue* attribute);
    static void DeleteVector (vector<NameSemanticValue*>* v);
    /**
     * Pretty print for a attribute stored as NameSemanticValue
     * @param ostr where to print
     * @param nameSemanticValue what to print
     * @return where to print next
     */
    friend ostream& operator<< (ostream& ostr, 
				const NameSemanticValue& nameSemanticValue);

private:
    /**
     * The name of the attribute
     */
    const char* m_name;
    /**
     * The type of the attribute
     */
    AttributeType::Enum m_type;
    /**
     * The value of the attribute
     */
    EvolverData::parser::semantic_type m_semanticValue;
};

#endif //__NAME_SEMANTIC_VALUE_H__

// Local Variables:
// mode: c++
// End:
