#ifndef __OTHER_ATTRIBUTES_H__
#define __OTHER_ATTRIBUTES_H__

#include <vector>
#include "lexYacc.h"
#include "Attribute.h"
using namespace std;

class IntegerAttribute : public Attribute
{
public:
    IntegerAttribute (int value) : m_value (value) {}
private:
    int m_value;
};

class RealAttribute : public Attribute
{
public:
    RealAttribute (float value) : m_value (value) {}
    virtual ~RealAttribute () {}
private:
    float m_value;
};


class IntegerArrayAttribute : public Attribute
{
public:
    /**
     * copies values vector
     */
    IntegerArrayAttribute (vector<float>& values)
    {
	m_values = new vector<int> (values.begin (), values.end ());
    }
    virtual ~IntegerArrayAttribute () {delete m_values;}
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
private:
    vector<float>* m_values;
};

#endif
// Local Variables:
// mode: c++
// End:
