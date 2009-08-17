#ifndef __NAME_SEMANTIC_ELEMENT_H__
#define __NAME_SEMANTIC_ELEMENT_H__

#include <Qt>
#include <vector>
#include "lexYacc.h"

class NameSemanticValue
{
public:
    enum Type
    {
        INT_SEMANTIC_TYPE,
        REAL_SEMANTIC_TYPE,
        COLOR_SEMANTIC_TYPE,
        INT_ARRAY_SEMANTIC_TYPE,
        REAL_ARRAY_SEMANTIC_TYPE,
        SEMANTIC_TYPE_COUNT
    };
    NameSemanticValue (const char* name, int i) : m_name(name)
    {
        m_semanticValue.m_int = i;
        m_type = INT_SEMANTIC_TYPE;
    }
	NameSemanticValue (const char* name, Color::Name color) : m_name(name)
    {
        m_semanticValue.m_color = color;
        m_type = COLOR_SEMANTIC_TYPE;
    }
    NameSemanticValue (const char* name, float r) : m_name(name)
    {
        m_semanticValue.m_real = r;
        m_type = REAL_SEMANTIC_TYPE;
    }
    NameSemanticValue (const char* name, vector<int>* intList) : m_name(name)
    {
        m_semanticValue.m_intList = intList;
        m_type = INT_ARRAY_SEMANTIC_TYPE;
    }
    NameSemanticValue (const char* name, vector<float>* realList) : m_name(name)
    {
        m_semanticValue.m_realList = realList;
        m_type = REAL_ARRAY_SEMANTIC_TYPE;
    }


    const char* GetName () const
    {
        return m_name;
    }
	const SemanticValue GetSemanticValue () const
    {
        return m_semanticValue;
    }
    Type GetType () const
    {
        return m_type;
    }
    
    vector<NameSemanticValue*>* PushBack (
        vector<NameSemanticValue*>* listSoFar);
    static void DeleteVector (vector<NameSemanticValue*>* v);

    friend ostream& operator<< (ostream& ostr, Type& type);
    friend ostream& operator<< (ostream& ostr, 
                                NameSemanticValue& nameSemanticValue);

private:
    const char* m_name;
    Type m_type;
    SemanticValue m_semanticValue;
};


#endif
