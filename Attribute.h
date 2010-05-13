/**
 * @file Attribute.h
 * @author Dan R. Lipsa
 * Contains declaration  of classes for all types  of attributes which
 * can be attached to vertices, edges, faces and bodies.
 */
#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

#include "Enums.h"

/**
 * Base class for all attributes
 */
class Attribute
{
public:
    /**
     * Virtual destructor for attributes (so that everything gets deleted from
     * derived classes)
     */
    virtual ~Attribute () {}
    /**
     * Virtual function that knows how to print an attribute
     * @param ostr where to print
     * @return where to print something else
     */
    virtual ostream& Print(ostream& ostr) const = 0;
};
/**
 * An integer attribute
 */
class IntegerAttribute : public Attribute
{
public:
    /**
     * Constructor
     * @param value the value of the attribute
     */
    IntegerAttribute (int value) : m_value (value) {}
    /**
     * Pretty prints an integer attribute 
     */
    virtual ostream& Print (ostream& ostr) const
    {
        return ostr << m_value;
    }
private:
    /**
     * Value of the integer attribute
     */
    int m_value;
};
/**
 * A real attribute
 */
class RealAttribute : public Attribute
{
public:
    /**
     * Constructor for the attribute with a real value
     * @param value the value of the attribute
     */
    RealAttribute (float value) : m_value (value) {}
    /**
     * Destructor for the real attribute
     */
    virtual ~RealAttribute () {}
    /**
     * Pretty print the attribute
     * @param ostr stream where to print
     * @return stream where we printed
     */
    virtual ostream& Print (ostream& ostr) const
    {return ostr << m_value;}
    
private:
    /**
     * The value of the attribute
     */
    float m_value;
};

/**
 * A color attribute
 */
class ColorAttribute : public Attribute
{
public:
    /**
     * Constructor for the color attribute
     * @param color the value of the attribute
     */
    ColorAttribute (Color::Name color) : m_color (color) {}
    /**
     * Pretty print
     * @param ostr where to print
     * @return where we printed
     */
    virtual ostream& Print (ostream& ostr) const
    {return ostr << m_color;}
    /**
     * Get the color of the attribute
     * @return color value of the attribute
     */
    Color::Name GetColor () const
    {return m_color;}
private:
    /**
     * Color value of the attribute
     */
    Color::Name m_color;
};

/**
 * An attribute that stores an array of integers
 */
class IntegerArrayAttribute : public Attribute
{
public:
    /**
     * Constructs an integer array attribute
     * @param values pointer to vector of integers.
     * WARNING: it takes ownership of the values vector.
     */
    IntegerArrayAttribute (vector<int>* values)
    {
        m_values = values;
    }
    /**
     * Destructor for the integer array attribute
     */
    virtual ~IntegerArrayAttribute () {delete m_values;}
    /**
     * Pretty print an array of integers attributes
     * @param ostr where to print
     * @return where to print the next item
     */
    virtual ostream& Print (ostream& ostr) const;
private:
    /**
     * Stores a pointer to an array of integers
     */
    vector<int>* m_values;
};
/**
 * An attribute that stores an array of reals
 */
class RealArrayAttribute : public Attribute
{
public:
    /**
     * Constructs an attribute that stores an array of reals
     * @param values pointer to an array of reals.
     * WARNING: Takes ownership of values vector
     */
    RealArrayAttribute (vector<float>* values)
    {
        m_values = values;
    }
    /**
     * Destructor
     */
    virtual ~RealArrayAttribute () {delete m_values;}
    /**
     * Pretty prints the attribute
     * @param ostr where to print
     * @return where we printed
     */
    virtual ostream& Print (ostream& ostr) const;
private:
    /**
     * Pointer to a vector of reals
     */
    vector<float>* m_values;
};
/**
 * Knows how to print an Attribute
 */
ostream& operator<< (ostream& ostr,  Attribute& attribute);

#endif //__ATTRIBUTE_H__

// Local Variables:
// mode: c++
// End:
