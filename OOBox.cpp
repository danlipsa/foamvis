/**
 * @file   OOBox.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the OOBox class
 */

#include "Debug.h"
#include "Edge.h"
#include "OOBox.h"
#include "Utils.h"
#include "Vertex.h"

// Private Classes
// ======================================================================
class LessThanDistanceFrom
{
public:
    LessThanDistanceFrom (const G3D::Vector3& begin) :
	m_begin (begin)
    {}
    bool operator() (const G3D::Vector3& x1, const G3D::Vector3& x2)
    {
	return (x1 - m_begin).squaredLength () < (x2 - m_begin).squaredLength ();
    }
private:
    G3D::Vector3 m_begin;
    
};


// Static Fields
// ======================================================================

// Methods
// ======================================================================
OOBox::OOBox (const G3D::Vector3& x, const G3D::Vector3& y,
	      const G3D::Vector3& z)
{
    m_vector[0] = x;
    m_vector[1] = y;
    m_vector[2] = z;
}

G3D::Vector3 OOBox::TorusTranslate (
    const G3D::Vector3& v, const G3D::Vector3int16& translation) const
{
    G3D::Vector3 ret = v;
    for (int i = 0; i < 3; i++)
	ret += (*this)[i] * translation[i];
    return ret;
}

OOBox::Intersections OOBox::Intersect (
    const G3D::Vector3& begin, const G3D::Vector3& end,
    const G3D::Vector3int16& beginLocation,
    const G3D::Vector3int16& endLocation) const
{
    using G3D::Vector3;using G3D::Line;using G3D::Plane;using G3D::Vector3int16;
    Vector3int16 translation = endLocation - beginLocation;
    size_t size = CountIntersections (translation) + 2;
    OOBox::Intersections intersections (size);
    intersections[0] = begin;
    intersections[size - 1] = end;
    Line line = Line::fromTwoPoints (begin, end);
    size_t index = 1;
    BOOST_FOREACH (AxisOrder pt, GetAxisOrder ())
    {
	size_t axis = pt[2];
	if (translation[axis] == 0)
	    continue;
	Vector3 planeTranslation = (*this)[axis];
	Vector3 planeNormal = (*this)[pt[0]].cross((*this)[pt[1]]);
	Vector3 planePoint = beginLocation[axis] * planeTranslation;
	if (translation[axis] == 1)
	    planePoint += planeTranslation;
	Plane plane = Plane (planeNormal, planePoint);
	intersections[index++] = line.intersection (plane);
    }
    sort (intersections.begin (), intersections.end (), 
	  LessThanDistanceFrom (begin));
    return intersections;
}


G3D::Vector3int16 OOBox::GetTranslationFromOriginalDomain (const G3D::Vector3& point) const
{
    using G3D::Vector3int16;using G3D::Vector3;using G3D::Plane;
    Vector3int16 location;
    RuntimeAssert (bm::isfinite (point.x) && bm::isfinite (point.y),
		   "OOBox::GetTranslationFromOriginalDomain: Infinite or NaN point");
    BOOST_FOREACH (AxisOrder pt, GetAxisOrder ())
    {
	size_t axis = pt[2];
	Vector3 planeNormal = (*this)[pt[0]].cross((*this)[pt[1]]);
	Vector3 planePoint;
	Vector3 planeTranslation = (*this)[axis];
	Vector3int16 increment = Vector3int16Unit (axis);
	Plane plane = Plane (planeNormal, planePoint);
	while (! plane.halfSpaceContainsFinite (point))
	{
	    location -= increment;
	    planePoint -= planeTranslation;
	    plane = Plane (planeNormal, planePoint);
	}
	planeNormal = - planeNormal;
	planePoint += planeTranslation;
	plane = Plane(planeNormal, planePoint);
	while (! plane.halfSpaceContainsFinite (point))
	{


	    location += increment;
	    planePoint += planeTranslation;
	    plane = Plane (planeNormal, planePoint);
	}
    }
    return location;
}

G3D::Matrix3 OOBox::GetMatrix () const
{
    return MatrixFromColumns ((*this)[0],(*this)[1],(*this)[2]);
}

G3D::Vector3int16 OOBox::GetTranslation (
    const G3D::Vector3& source, const G3D::Vector3& destination) const
{
    using G3D::Matrix3;using G3D::Vector3;using G3D::Vector3int16;
    Matrix3 toOrthonormal = GetMatrix ().inverse ();
    Vector3 s = toOrthonormal * source;
    Vector3 d = toOrthonormal * destination;
    Vector3 t = d - s - Vector3 (0.5, 0.5, 0.5);
    Vector3int16 translation (ceil (t.x), ceil(t.y), ceil (t.z));
    return translation;
}

bool OOBox::IsZero () const
{
    return GetX ().isZero () && GetY ().isZero () && GetZ ().isZero ();
}

bool OOBox::IsTorus () const
{
    return ! IsZero ();
}


bool OOBox::IsWrap (const G3D::Vector3& begin, const G3D::Vector3& end,
		    G3D::Vector3int16* translation) const
{
    G3D::Vector3int16 t;
    G3D::Vector3 u = GetMatrix ().inverse () * (end - begin);
    for (size_t i = 0; i < 3; ++i)
    {
	if (abs (u[i]) > 0.5)
	{
	    if (u[i] > 0)
		t += Vector3int16Unit (i);
	    else
		t -= Vector3int16Unit (i);
	}
    }
    if (translation != 0 && t != Vector3int16Zero)
    {
	*translation = t;
	return true;
    }
    else
	return false;
}


// Static and Friends Methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OOBox& box)
{
    return ostr << box.GetX () << endl << box.GetY () << endl 
		<< box.GetZ () << endl;
}

size_t OOBox::CountIntersections (const G3D::Vector3int16& location)
{
    return ((location.x != 0) + (location.y != 0) + (location.z != 0));
}
