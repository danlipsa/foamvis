/**
 * @file AffineMap.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the AffineMap class
 */
#ifndef __AFFINE_MAP_H__
#define __AFFINE_MAP_H__

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
	return 3;
    }
    G3D::Vector2 GetTranslation () const
    {
	return m_translation;
    }
    float GetAngle () const
    {
	return m_angle;
    }
    void Set (size_t i, double value);
private:
    G3D::Vector2 m_translation;
    float m_angle;
};

#endif //__AFFINE_MAP_H__

// Local Variables:
// mode: c++
// End:
