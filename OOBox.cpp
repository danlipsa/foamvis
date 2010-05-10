/**
 * @file   OOBox.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the OOBox class
 */

#include "Debug.h"
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


// Static Fields
// ======================================================================
const boost::array<OOBox::NormalPoint,3> OOBox::PLANES =
{{
	{{0, 1}},
	{{1, 2}},
	{{2, 0}}
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
    const G3D::Vector3int16& domainIncrement) const
{
    G3D::Vector3 ret = v;
    for (int i = 0; i < 3; i++)
	ret += (*this)[i] * domainIncrement[i];
    return ret;
}

OOBox::Intersections OOBox::Intersect (
    const G3D::Vector3& begin, const G3D::Vector3& end,
    const G3D::Vector3int16& domainIncrement) const
{
    using G3D::Vector3;using G3D::Line;using G3D::Plane;
    size_t size = CountIntersections (domainIncrement) + 2;
    OOBox::Intersections intersections;
    intersections.resize (size);
    intersections[0] = begin;
    intersections[size - 1] = end;
    Vector3 planeNormal, planePoint;
    Plane plane;
    Line line = Line::fromTwoPoints (begin, end);
    BOOST_FOREACH (NormalPoint np, OOBox::PLANES)
    {
	if (domainIncrement[np[0]] == 0)
	    continue;
	planeNormal = (*this)[np[0]];
	planePoint = (*this)[np[1]];
	if (domainIncrement[np[0]] == 1)
	    planePoint += planeNormal;
	plane = Plane (planeNormal, planePoint);
	intersections[np[0] + 1] = line.intersection (plane);
    }
    sort (intersections.begin (), intersections.end (), 
	  LessThanDistanceFrom (begin));
    return intersections;
}


G3D::Vector3int16 OOBox::GetTorusLocation (const G3D::Vector3& point) const
{
    using G3D::Vector3int16;
    using G3D::Vector3;
    using G3D::Plane;
    Vector3 planeNormal, planePoint;
    Plane plane;
    Vector3int16 domainIncrement;
    BOOST_FOREACH (NormalPoint np, OOBox::PLANES)
    {
	planeNormal = (*this)[np[0]];
	planePoint = (*this)[np[1]];
	Vector3int16 increment = Vertex::UnitVector3int16 (np[0]);
	plane = Plane (planeNormal, planePoint);
	if (! plane.halfSpaceContainsFinite (point))
	    domainIncrement -= increment;;
	plane = Plane(-planeNormal, planePoint + planeNormal);
	if (! plane.halfSpaceContainsFinite (point))
	    domainIncrement += increment;;
    }
    return domainIncrement;
}


// Static and Friends Methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OOBox& box)
{
    return ostr << box.GetX () << endl << box.GetY () << endl 
		<< box.GetZ () << endl;
}

size_t OOBox::CountIntersections (const G3D::Vector3int16& domainIncrement)
{
    return ((domainIncrement.x != 0) + 
	    (domainIncrement.y != 0) + (domainIncrement.z != 0));
}
