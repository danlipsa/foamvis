/**
 * @file   OOBox.h
 * @author Dan R. Lipsa
 * @date 7 May 2010
 * 
 * Declaration of the OOBox class
 */
#ifndef __OOBOX_H__
#define __OOBOX_H__

class OOBox
{
public:
    OOBox (const G3D::Vector3& x, const G3D::Vector3& y,
	   const G3D::Vector3& z) :
    m_x (x), m_y (y), m_z (z)
    {}
    OOBox () {}

    const G3D::Vector3& GetX () const
    {
	return m_x;
    }
    const G3D::Vector3& GetY () const
    {
	return m_y;
    }
    const G3D::Vector3& GetZ () const
    {
	return m_z;
    }
    void Set (const G3D::Vector3& x, const G3D::Vector3& y, 
	      const G3D::Vector3& z)
    {
	m_x = x;
	m_y = y;
	m_z = z;
    }
    const G3D::Vector3& operator[] (size_t i) const;
    G3D::Vector3 TorusTranslate (
	const G3D::Vector3& v, const G3D::Vector3int16& domainIncrement) const;
    G3D::Vector3 Intersect (
	const G3D::Vector3& begin, const G3D::Vector3& end, 
	const G3D::Vector3int16& domainIncrement) const;
    G3D::Vector3int16 GetTorusLocation (const G3D::Vector3& point) const;

public:
    friend ostream& operator<< (ostream& ostr, const OOBox& box);

private:
    G3D::Vector3 m_x;
    G3D::Vector3 m_y;
    G3D::Vector3 m_z;
};


#endif //__OOBOX_H__

// Local Variables:
// mode: c++
// End:
