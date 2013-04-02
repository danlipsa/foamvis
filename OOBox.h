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
    typedef vector<G3D::Vector3> Intersections;

public:
    OOBox (const G3D::Vector3& x, const G3D::Vector3& y,
	   const G3D::Vector3& z);
    OOBox () {}

    const G3D::Vector3& GetX () const
    {
	return m_vector[0];
    }
    const G3D::Vector3& GetY () const
    {
	return m_vector[1];
    }
    const G3D::Vector3& GetZ () const
    {
	return m_vector[2];
    }
    void Set (const G3D::Vector3& x, const G3D::Vector3& y, 
	      const G3D::Vector3& z)
    {
	m_vector[0] = x;
	m_vector[1] = y;
	m_vector[2] = z;
    }
    G3D::Matrix3 GetMatrix () const;

    bool IsZero () const;
    bool IsTorus () const;

    const G3D::Vector3& operator[] (size_t i) const
    {
	return m_vector[i];
    }
    G3D::Vector3 TorusTranslate (
	const G3D::Vector3& v, const G3D::Vector3int16& translation) const;
    Intersections Intersect (
	const G3D::Vector3& begin, const G3D::Vector3& end,
	const G3D::Vector3int16& beginLocation,
	const G3D::Vector3int16& endLocation) const;
    G3D::Vector3int16 GetLocation (const G3D::Vector3& point) const;
    G3D::Vector3int16 GetTranslation (
	const G3D::Vector3& source, const G3D::Vector3& destination) const;
    bool IsWrap (const G3D::Vector3& begin, const G3D::Vector3& end,
		 G3D::Vector3int16* translation = 0) const;

public:
    friend ostream& operator<< (ostream& ostr, const OOBox& box);
    static size_t CountIntersections (const G3D::Vector3int16& location);

private:
    boost::array<G3D::Vector3, 3> m_vector;
};


#endif //__OOBOX_H__

// Local Variables:
// mode: c++
// End:
