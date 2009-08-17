/**
 * @file Attribute.h
 * @author Dan R. Lipsa
 * Contains  declaration of classes for all types of attributes
 */
#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__
#include <iostream>
#include <vector>
#include <Qt>
#include "lexYacc.h"
using namespace std;
class QColor;

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
     *
     */
    virtual ostream& Print (ostream& ostr) const
    {
        return ostr << m_value;
    }
private:
    int m_value;
};

class RealAttribute : public Attribute
{
public:
    RealAttribute (float value) : m_value (value) {}
    virtual ~RealAttribute () {}
    virtual ostream& Print (ostream& ostr) const {return ostr << m_value;}
private:
    float m_value;
};

class ColorAttribute : public Attribute
{
public:
	ColorAttribute (Color::Name color) : m_color (color) {}
    virtual ostream& Print (ostream& ostr) const {return ostr << m_color;}
	Color::Name GetColor () const
    {return m_color;}
private:
	Color::Name m_color;
};


class IntegerArrayAttribute : public Attribute
{
public:
    /**
     * takes ownership of the values vector
     */
    IntegerArrayAttribute (vector<int>* values)
    {
        m_values = values;
    }
    virtual ~IntegerArrayAttribute () {delete m_values;}
    virtual ostream& Print (ostream& ostr) const;
private:
    vector<int>* m_values;
};

class RealArrayAttribute : public Attribute
{
public:
    /**
     * takes ownership of values vector
     */
    RealArrayAttribute (vector<float>* values)
    {
        m_values = values;
    }
    virtual ~RealArrayAttribute () {delete m_values;}
    virtual ostream& Print (ostream& ostr) const;
private:
    vector<float>* m_values;
};

ostream& operator<< (ostream& ostr, const Attribute& attribute);


#endif
// Local Variables:
// mode: c++
// End:
