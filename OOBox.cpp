/**
 * @file   OOBox.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the OOBox class
 */

#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "OOBox.h"
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

// Private Functions
// ======================================================================
void equalize (G3D::Vector3& first, G3D::Vector3& second)
{
    for (int i = 0; i < 3; i++)
	if (G3D::fuzzyEq (first[i], second[i]))
	    first[i] = second[i];
}



// Static Fields
// ======================================================================
const boost::array<OOBox::PlaneTranslation,3> OOBox::PLANES =
{{
	{{1, 2, 0}},
	{{2, 0, 1}},
	{{0, 1, 2}}
}};

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
    const G3D::Vector3& v,
    const G3D::Vector3int16& translation) const
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
    BOOST_FOREACH (PlaneTranslation pt, OOBox::PLANES)
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

    cdbg << "Intersections for begin=" << begin << ", end=" << end << endl;
    ostream_iterator<G3D::Vector3> o (cdbg, " ");
    copy (intersections.begin (), intersections.end (), o);
    cdbg << endl;
    return intersections;
}


G3D::Vector3int16 OOBox::GetLocation (const G3D::Vector3& point) const
{
    using G3D::Vector3int16;using G3D::Vector3;using G3D::Plane;
    Vector3int16 location;
    BOOST_FOREACH (PlaneTranslation pt, OOBox::PLANES)
    {
	size_t axis = pt[2];
	Vector3 planeNormal = (*this)[pt[0]].cross((*this)[pt[1]]);
	Vector3 planePoint;
	Vector3 planeTranslation = (*this)[axis];
	Vector3int16 increment = Vertex::UnitVector3int16 (axis);
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
	    // G3D bug: Vector3int16::operator+=
	    // location = location + increment;
	    location += increment;
	    planePoint += planeTranslation;
	    plane = Plane (planeNormal, planePoint);
	}
    }
    return location;
}

G3D::Vector3int16 OOBox::GetTranslation (
    const G3D::Vector3& source, const G3D::Vector3& destination) const
{
    using G3D::Matrix3;using G3D::Vector3;using G3D::Vector3int16;
    Matrix3 toOrthonormal, toPeriods;
    for (int i = 0; i < 3; i++)
	toPeriods.setColumn (i, (*this)[i]);
    toOrthonormal = toPeriods.inverse ();
    Vector3 o = toOrthonormal * source;
    Vector3 d = toOrthonormal * destination;
    equalize (o, d);
    Vector3int16 sourceLocation (floorf (o.x), floorf(o.y), floorf(o.z));
    Vector3int16 destinationLocation (floorf (d.x), floorf(d.y), floorf(d.z));
    return destinationLocation - sourceLocation;
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
