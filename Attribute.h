#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

#include <iostream>
#include <vector>
#include <Qt>
#include "lexYacc.h"
using namespace std;

class QColor;

class Attribute
{
public:
    virtual ~Attribute () {}
    virtual ostream& Print(ostream& ostr) const = 0;
};

class IntegerAttribute : public Attribute
{
public:
    IntegerAttribute (int value) : m_value (value) {}
    virtual ostream& Print (ostream& ostr) const {return ostr << m_value;}
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
    ColorAttribute (Qt::GlobalColor color) : m_color (color) {}
    virtual ostream& Print (ostream& ostr) const {return ostr << m_color;}
    Qt::GlobalColor GetColor () const
    {return m_color;}
private:
    Qt::GlobalColor m_color;
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
