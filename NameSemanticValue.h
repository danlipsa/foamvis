/**
 * @file   NameSemanticValue.h
 * @author Dan R. Lipsa
 *
 * Tupple (name, type, value) used for attributes
 */
#ifndef __NAME_SEMANTIC_VALUE_H__
#define __NAME_SEMANTIC_VALUE_H__
#include "EvolverData_yacc.h"

/**
 * Tupple (name, type, value) used for attributes
 */
class NameSemanticValue
{
public:
    /**
     * Types of attributes
     */
    enum Type
    {
        INT,
        REAL,
        COLOR,
        INT_ARRAY,
        REAL_ARRAY,
        COUNT
    };
    /**
     * Constructor for an attribute with type INT
     * @param name name of the attribute
     * @param i the value of the attribute
     */
    NameSemanticValue (const char* name, int i) : m_name(name)
    {
        m_semanticValue.m_int = i;
        m_type = INT;
    }
    /**
     * Constructor for an attribute with type COLOR
     * @param name name of the attribute
     * @param color color of the attribute
     */
    NameSemanticValue (const char* name, Color::Name color) : m_name(name)
    {
        m_semanticValue.m_color = color;
        m_type = COLOR;
    }
    /**
     * Constructor for an attribute with type REAL
     * @param name name of the attribute
     * @param r value of the attribute
     */
    NameSemanticValue (const char* name, float r) : m_name(name)
    {
        m_semanticValue.m_real = r;
        m_type = REAL;
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
        m_type = INT_ARRAY;
    }
    /**
     * Constructor for an attribute with type REAL_ARRAY
     * @param name name of the attribute
     * @param realList value of the attribute
     */
    NameSemanticValue (const char* name, vector<float>* realList) : 
        m_name(name)
    {
        m_semanticValue.m_realList = realList;
        m_type = REAL_ARRAY;
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
    const EvolverData::parser::semantic_type GetSemanticValue () const
    {
        return m_semanticValue;
    }
    /**
     * Gets the type of the attribute
     * @return the type of the attribute
     */
    Type GetType () const
    {
        return m_type;
    }
    /**
     * Adds this attribute to a list of attributes
     * @param listSoFar where to add this attribute
     * @return the new list of attributes
     */
    vector<NameSemanticValue*>* PushBack (
        vector<NameSemanticValue*>* listSoFar);
    /**
     * Deletes a vector  of attributes and the elements  of the vector
     * as well (deep delete).
     * @param v the vector to be deleted
     */
    static void DeleteVector (vector<NameSemanticValue*>* v);
    /**
     * Pretty print for a Type
     * @param ostr where to print
     * @param type what to print
     * @return where to print next
     */
    friend ostream& operator<< (ostream& ostr, Type& type);
    /**
     * Pretty print for a attribute stored as NameSemanticValue
     * @param ostr where to print
     * @param nameSemanticValue what to print
     * @return where to print next
     */
    friend ostream& operator<< (ostream& ostr, 
				NameSemanticValue& nameSemanticValue);

private:
    /**
     * The name of the attribute
     */
    const char* m_name;
    /**
     * The type of the attribute
     */
    Type m_type;
    /**
     * The value of the attribute
     */
    EvolverData::parser::semantic_type m_semanticValue;
};

#endif //__NAME_SEMANTIC_VALUE_H__

// Local Variables:
// mode: c++
// End:
