#ifndef __NAME_SEMANTIC_ELEMENT_H__
#define __NAME_SEMANTIC_ELEMENT_H__

#include <vector>
#include <iostream>
#include "EvolverData_yacc.h"

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
	NameSemanticValue (const char* name, std::vector<int>* intList) : m_name(name)
    {
        m_semanticValue.m_intList = intList;
        m_type = INT_ARRAY_SEMANTIC_TYPE;
    }
	NameSemanticValue (const char* name, std::vector<float>* realList) : m_name(name)
    {
        m_semanticValue.m_realList = realList;
        m_type = REAL_ARRAY_SEMANTIC_TYPE;
    }


    const char* GetName () const
    {
        return m_name;
    }
	const EvolverData::parser::semantic_type GetSemanticValue () const
    {
        return m_semanticValue;
    }
    Type GetType () const
    {
        return m_type;
    }
    
	std::vector<NameSemanticValue*>* PushBack (
		std::vector<NameSemanticValue*>* listSoFar);
	static void DeleteVector (std::vector<NameSemanticValue*>* v);

	friend std::ostream& operator<< (std::ostream& ostr, Type& type);
	friend std::ostream& operator<< (std::ostream& ostr, 
                                NameSemanticValue& nameSemanticValue);

private:
    const char* m_name;
    Type m_type;
    EvolverData::parser::semantic_type m_semanticValue;
};

#endif //__NAME_SEMANTIC_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
