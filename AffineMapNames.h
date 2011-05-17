/**
 * @file Foam.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the AffineMapNames class
 */
#ifndef __AFFINE_MAP_NAMES_H__
#define __AFFINE_MAP_NAMES_H__

/**
 * Names of parameters in a DMP file where an affine transformation for a 
 * foam object is stored.
 * Names for x, y and angle are stored.
 */
class AffineMapNames
{
public:
    size_t size () const
    {
	return m_names.size ();
    }
    bool IsEmpty () const
    {
	return X ().empty ();
    }
    string X () const
    {
	return m_names[0];
    }
    string Y () const
    {
	return m_names[1];
    }
    string Angle () const
    {
	return m_names[2];
    }
    void Set (size_t i, const string& name)
    {
	m_names[i] = name;
    }
    string Get (size_t i) const
    {
	return m_names[i];
    }

private:
    boost::array<string, 3> m_names;
};

struct AffineMap
{
public:
    size_t size () const
    {
	return m_values.size ();
    }
    double X () const
    {
	return m_values[0];
    }
    double Y () const
    {
	return m_values[1];
    }
    double Angle () const
    {
	return m_values[3];
    }
    void Set (size_t i, double value)
    {
	m_values[i] = value;
    }
    double Get (size_t i) const
    {
	return m_values[i];
    }
private:
    boost::array<double, 3> m_values;
};

#endif //__AFFINE_MAP_NAMES_H__

// Local Variables:
// mode: c++
// End:
