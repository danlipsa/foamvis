#ifndef __ATTRIBUTE_CREATOR_H__
#define __ATTRIBUTE_CREATOR_H__

#include "Attribute.h"
#include "lexYacc.h"

class AttributeCreator
{
public:
    virtual Attribute* operator () (SemanticValue& value) = 0;
};

class IntegerAttributeCreator : public AttributeCreator
{
public:
    virtual Attribute* operator() (SemanticValue& value);
};

class RealAttributeCreator : public AttributeCreator
{
public:
    virtual Attribute* operator() (SemanticValue& value);
};

class IntegerArrayAttributeCreator : public AttributeCreator
{
public:
    IntegerArrayAttributeCreator (int size) : m_size (size) {}
    virtual Attribute* operator() (SemanticValue& value);
private:
    unsigned int m_size;
};

class RealArrayAttributeCreator : public AttributeCreator
{
public:
    RealArrayAttributeCreator (int size) : m_size (size) {}
    virtual Attribute* operator() (SemanticValue& value);
private:
    unsigned int m_size;
};


#endif
// Local Variables:
// mode: c++
// End:
