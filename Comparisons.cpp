/**
 * @file   Comparisons.cpp
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Implementation comparison functors
 */

#include "Comparisons.h"
#include "Vertex.h"
#include "Edge.h"
#include "OrientedFace.h"
#include "OrientedFaceIndex.h"


// ======================================================================
// VectorLessThanAngle

bool VectorLessThanAngle::operator () (
    const G3D::Vector3& first, const G3D::Vector3& second) const
{
    using G3D::Vector3;
    double firstAngle = angle0pi (first, m_originNormal);
    double secondAngle = angle0pi (second, m_originNormal);
    if (firstAngle < secondAngle)
	return true;
    else if (firstAngle == secondAngle)
    {
	Vector3 normal1, normal2;
	m_originNormal.getTangents (normal1, normal2);
	if (angle (first, normal1) < angle (second, normal1))
	    return true;
	else
	    return false;
    }
    else
	return false;
}

double VectorLessThanAngle::angle (
    const G3D::Vector3& first, const G3D::Vector3& second)
{
    double angle = acos (first.dot (second));
    double sinValue = first.cross (second).length ();
    if (sinValue < 0)
	angle = 2*M_PI - angle;
    return angle;
}


// ======================================================================
// VectorLessThan

bool VertexLessThan::operator () (
    const boost::shared_ptr<Vertex>& first,
    const boost::shared_ptr<Vertex>& second) const
{
    return *first < *second;
}

bool EdgeLessThan::operator () (const Edge* first, const Edge* second) const
{
    return *first < *second;
}

// ======================================================================
// VectorLessThanAlong

bool VertexLessThanAlong::operator() (
    const boost::shared_ptr<Vertex>& first,
    const boost::shared_ptr<Vertex>& second) const
{
    return operator() (static_cast<const G3D::Vector3*>(first.get ()),
		       static_cast<const G3D::Vector3*>(second.get ()));
}

// ======================================================================
// OrientedFaceIndexLessThan

bool OrientedFaceIndexLessThan::operator () (
    const OrientedFaceIndex& first, const OrientedFaceIndex& second) const
{
    return first.GetOrientedFace ()->GetId () <
	second.GetOrientedFace ()->GetId ();
}
