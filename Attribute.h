/**
 * @file Attribute.h
 * @author Dan R. Lipsa
 * Contains declaration  of classes for all types  of attributes which
 * can be attached to vertices, edges, faces and bodies.
 */
#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

#include "ParsingEnums.h"

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
    virtual ostream& Print(ostream& ostr) const 
    {
	return ostr << "Attribute placeholder" << endl;
    }
};
/**
 * An integer attribute
 */
class IntegerAttribute : public Attribute
{
public:
    typedef int value_type;
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
    operator int()
    {
	return m_value;
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
    typedef double value_type;
    /**
     * Constructor for the attribute with a real value
     * @param value the value of the attribute
     */
    RealAttribute (double value) : m_value (value) {}
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
    operator double ()
    {
	return m_value;
    }
    void set (double value)
    {
	m_value = value;
    }
    
private:
    /**
     * The value of the attribute
     */
    double m_value;
};

/**
 * A color attribute
 */
class ColorAttribute : public Attribute
{
public:
    typedef Color::Enum value_type;
    /**
     * Constructor for the color attribute
     * @param color the value of the attribute
     */
    ColorAttribute (Color::Enum color) : m_color (color) {}
    /**
     * Pretty print
     * @param ostr where to print
     * @return where we printed
     */
    virtual ostream& Print (ostream& ostr) const
    {
	return ostr << m_color;
    }
    /**
     * Get the color of the attribute
     * @return color value of the attribute
     */
    operator Color::Enum ()
    {
	return m_color;
    }
    void set (Color::Enum color)
    {
	m_color = color;
    }
private:
    /**
     * Color value of the attribute
     */
    Color::Enum m_color;
};

/**
 * An attribute that stores an array of integers
 */
class IntegerArrayAttribute : public Attribute
{
public:
    typedef const vector<int> value_type;
    /**
     * Constructs an integer array attribute
     * @param values pointer to vector of integers.
     * WARNING: it takes ownership of the values vector.
     */
    IntegerArrayAttribute (vector<int>* values)
    {
        m_values.reset (values);
    }
    IntegerArrayAttribute (value_type values)
    {
	m_values.reset (new vector<int> (values));
    }
    void set (value_type values)
    {
	*m_values = values;
    }
    /**
     * Pretty print an array of integers attributes
     * @param ostr where to print
     * @return where to print the next item
     */
    virtual ostream& Print (ostream& ostr) const;
    operator const vector<int> ()
    {
	return *m_values;
    }

private:
    /**
     * Stores a pointer to an array of integers
     */
    boost::shared_ptr< vector<int> > m_values;
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
    RealArrayAttribute (vector<double>* values)
    {
        m_values.reset (values);
    }
    /**
     * Pretty prints the attribute
     * @param ostr where to print
     * @return where we printed
     */
    virtual ostream& Print (ostream& ostr) const;
private:
    boost::shared_ptr< vector<double> > m_values;
};

/**
 * An attribute that stores an array of attributes (used for
 * multidimensional arrays)
 */
class AttributeArrayAttribute : public Attribute
{
public:
    /**
     * Constructs an attribute that stores an array of attributes
     */
    AttributeArrayAttribute ();
    // takes ownership of the pointer.
    AttributeArrayAttribute (Attribute* element);
    AttributeArrayAttribute (size_t n, double value);
    AttributeArrayAttribute (size_t n);
    
    void CheckDimensions (vector<size_t>* dimensions);
    static AttributeArrayAttribute* NewArray (vector<size_t>* dimensions);
    double Get (const vector<size_t>& index) const;
    void AddElement (Attribute* element)
    {
	m_values->push_back (boost::shared_ptr<Attribute> (element));
    }

    /**
     * Pretty prints the attribute
     * @param ostr where to print
     * @return where we printed
     */
    virtual ostream& Print (ostream& ostr) const;
    size_t size ()
    {
	return m_values->size ();
    }

private:
    void setElement (size_t i, Attribute* element)
    {
	(*m_values)[i] = boost::shared_ptr<Attribute> (element);
    }
    boost::shared_ptr<Attribute> getElement (size_t i) const
    {
	return (*m_values)[i];
    }
    void checkDimensions (vector<size_t>* dimensions,
			  size_t currentDimensionIndex);
    static AttributeArrayAttribute* newArray (vector<size_t>* dimensions,
					      size_t currentDimensionIndex);
private:
    /**
     * Pointer to a vector of reals
     */
    boost::shared_ptr< vector<boost::shared_ptr<Attribute> > > m_values;
};


/**
 * Knows how to print an Attribute
 */
ostream& operator<< (ostream& ostr, const Attribute& attribute);
ostream& operator<< (ostream& ostr, const Attribute* attribute);

#endif //__ATTRIBUTE_H__

// Local Variables:
// mode: c++
// End:
