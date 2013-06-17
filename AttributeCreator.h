/**
 * @file   AttributeCreator.h
 * @author Dan R. Lipsa
 * @brief Create attributes which can be attached to 
 *        vertices, edges, faces and bodies.
 * 
 */
#ifndef __ATTRIBUTE_CREATOR_H__
#define __ATTRIBUTE_CREATOR_H__

#include "Enums.h"
#include "EvolverData_yacc.h"

class Attribute;
class IntegerAttribute;

/**
 * @brief Create attributes which can be attached to 
 *        vertices, edges, faces and bodies.
 */
class AttributeCreator
{
public:
    /**
     * Pure virtual function used to create an attribute
     * @param value value of the attribute
     * @param type type of the attribute
     * @return a pointer to the created attribute
     */
    virtual Attribute* operator () (
	const EvolverData::parser::semantic_type& value, 
        AttributeType::Enum type) const = 0;
};

/**
 * @brief Creates an integer attribute.
 */
class IntegerAttributeCreator : public AttributeCreator
{
public:
    /**
     * Creates an integer attribute
     * @param value of the integer attribute
     * @param type of the attribute. Has to be NameSemanticValue::INT.
     * @return a pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value,
        AttributeType::Enum type) const;
};

/**
 * @brief Creates a color attribute.
 */
class ColorAttributeCreator : public AttributeCreator
{
public:
    /**
     * Creates a color attribute
     * @param value color of the attribute
     * @param type needs to be NameSemanticValue::COLOR
     * @return a pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value,
        AttributeType::Enum type) const;
};
/**
 * @brief Creates a real attribute.
 */
class RealAttributeCreator : public AttributeCreator
{
public:
    /**
     * Creates a real attribute
     * @param value real value of the attribute
     * @param type can be REAL or INT
     * @return a pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value, 
        AttributeType::Enum type) const;
};
/**
 * @brief Creates an attribute which stores an integer array 
 */
class IntegerArrayAttributeCreator : public AttributeCreator
{
public:
    /**
     * Constructor for the functor
     * @param size the size of the array
     */
    IntegerArrayAttributeCreator (int size) : 
	m_size (size) 
    {}
    IntegerArrayAttributeCreator () : 
	m_size (0) 
    {}

    /**
     * Creates an attribute which stores an integer array
     * @param value an integer list stored in a m_intList
     * @param type has to be NameSemanticValue::INT_ARRAY
     * @return pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value,
        AttributeType::Enum type) const;
private:
    /**
     * Size of the array of integers
     */
    size_t m_size;
};


/**
 * @brief Creates an attribute that stores an integer vector.
 *
 * Similar with an IntegerArrayAttributeCreator but does not store the
 * size of the array. Used for edge and vertex CONSTRAINTS
 */
class IntegerVectorAttributeCreator : public AttributeCreator
{
public:
    /**
     * Creates an attribute which stores an integer array
     * @param value an integer list stored in a m_intList
     * @param type has to be NameSemanticValue::INT_ARRAY
     * @return pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value,
        AttributeType::Enum type) const;
};
/**
 * @brief Creates an attribute that stores an array of real values
 */
class RealArrayAttributeCreator : public AttributeCreator
{
public:
    /**
     * Constructor for the functor
     * @param size size of the array of real values
     */
    RealArrayAttributeCreator (int size) : m_size (size) {}
    /**
     * Creates an attribute that stores an array of real values
     * @param value stores a pointer to a real list in a m_realList
     * @param type has to be NameSemanticValue::REAL_ARRAY
     * @return pointer to the created attribute
     */
    virtual Attribute* operator() (
	const EvolverData::parser::semantic_type& value,
        AttributeType::Enum type) const;
private:
    /**
     * Size of the array of reals
     */
    size_t m_size;
};

#endif //__ATTRIBUTE_CREATOR_H__

// Local Variables:
// mode: c++
// End:
