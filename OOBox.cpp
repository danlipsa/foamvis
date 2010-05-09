/**
 * @file   OOBox.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the OOBox class
 */

#include "OOBox.h"
#include "Debug.h"

const G3D::Vector3& OOBox::operator[] (size_t i) const
{
    switch (i)
    {
    case 0:
	return m_x;
    case 1:
	return m_y;
    case 2:
	return m_z;
    default:
	RuntimeAssert (false, "Invalid index for OOBox");
	return m_x;
    }
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

G3D::Vector3 OOBox::Intersect (
    const G3D::Vector3& begin, const G3D::Vector3& end,
    const G3D::Vector3int16& domainIncrement) const
{
    using G3D::Line;
    Line line = Line::fromTwoPoints (begin, end);

}

G3D::Vector3int16 OOBox::GetTorusLocation (const G3D::Vector3& point) const
{
    using G3D::Vector3int16;
    using G3D::Vector3;
    using G3D::Plane;
    typedef pair<Vector3int16, Vector3int16> PlanePair;
    boost::array<PlanePair, 3> planePairs = {{
	    PlanePair (Vector3int16 (2, 1, 0), Vector3int16 (1, 0, 0)),// X
	    PlanePair (Vector3int16 (0, 2, 1), Vector3int16 (0, 1, 0)),// Y
	    PlanePair (Vector3int16 (1, 0, 2), Vector3int16 (0, 0, 1)) // Z
	}};
    Vector3 one, two, three, translate;
    Plane p;
    Vector3int16 domainIncrement;
    BOOST_FOREACH (PlanePair planePair, planePairs)
    {
	Vector3int16 plane = planePair.first;
	Vector3int16 increment = planePair.second;
	one = (*this)[plane.x];
	two = Vector3::zero ();
	three = (*this)[plane.y];
	translate = (*this)[plane.z];
	p = Plane (one, two, three);
	if (! p.halfSpaceContainsFinite (point))
	    domainIncrement -= increment;;
	p = Plane(three + translate, two + translate, one + translate);
	if (! p.halfSpaceContainsFinite (point))
	    domainIncrement += increment;;
    }
    return domainIncrement;
}




// Static and Friends Methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OOBox& box)
{
    return ostr << box.m_x << endl << box.m_y << endl << box.m_z << endl;
}
