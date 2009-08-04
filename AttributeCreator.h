#ifndef __ATTRIBUTE_CREATOR_H__
#define __ATTRIBUTE_CREATOR_H__

#include "Attribute.h"
#include "lexYacc.h"
#include "NameSemanticValue.h"

class AttributeCreator
{
public:
    virtual Attribute* operator () (
	const SemanticValue& value, NameSemanticValue::Type type) = 0;
};

class IntegerAttributeCreator : public AttributeCreator
{
public:
    virtual Attribute* operator() (const SemanticValue& value,
				   NameSemanticValue::Type type);
};

class ColorAttributeCreator : public AttributeCreator
{
public:
    virtual Attribute* operator() (const SemanticValue& value,
				   NameSemanticValue::Type type);
};


class RealAttributeCreator : public AttributeCreator
{
public:
    virtual Attribute* operator() (const SemanticValue& value, 
				   NameSemanticValue::Type type);
};

class IntegerArrayAttributeCreator : public AttributeCreator
{
public:
    IntegerArrayAttributeCreator (int size) : m_size (size) {}
    virtual Attribute* operator() (const SemanticValue& value,
				   NameSemanticValue::Type type);
private:
    unsigned int m_size;
};

class RealArrayAttributeCreator : public AttributeCreator
{
public:
    RealArrayAttributeCreator (int size) : m_size (size) {}
    virtual Attribute* operator() (const SemanticValue& value,
				   NameSemanticValue::Type type);
private:
    unsigned int m_size;
};


#endif
// Local Variables:
// mode: c++
// End:
