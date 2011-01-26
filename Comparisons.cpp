/**
 * @file   Comparisons.cpp
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Implementation comparison functors
 */
#include "Body.h"
#include "Comparisons.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
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
// VertexLessThan

bool VertexLessThan::operator () (
    const boost::shared_ptr<const Vertex>& first,
    const boost::shared_ptr<const Vertex>& second) const
{
    return *first < *second;
}


// ======================================================================
// EdgeLessThan

bool EdgeLessThan::operator () (
    const boost::shared_ptr<const Edge>& first,
    const boost::shared_ptr<const Edge>& second) const
{
    return *first < *second;
}

// ======================================================================
// FaceLessThan

bool FaceLessThan::operator () (
    const boost::shared_ptr<const Face>& first,
    const boost::shared_ptr<const Face>& second) const
{
    return *first < *second;
}



// ======================================================================
// VertexLessThanAlong

bool VertexLessThanAlong::operator() (
    const boost::shared_ptr<Vertex>& first,
    const boost::shared_ptr<Vertex>& second) const
{
    return operator() (*first, *second);
}

double VertexLessThanAlong::operator() (
    const boost::shared_ptr<Vertex>& x) const
{
    return operator() (*x);
}


// ======================================================================
// OrientedFaceIndexLessThan

bool OrientedFaceIndexLessThan::operator () (
    const OrientedFaceIndex& first, const OrientedFaceIndex& second) const
{
    boost::shared_ptr<OrientedFace> firstOf = first.GetOrientedFace ();
    boost::shared_ptr<OrientedFace> secondOf = second.GetOrientedFace ();
    return firstOf->GetId () <	secondOf->GetId ();
}

bool BodyLessThan (const boost::shared_ptr<Body>& first,
		   const boost::shared_ptr<Body>& second)
{
    return *first < *second;
}

// ======================================================================
// BBObjectLessThanAlong

template <typename BBObject>
bool BBObjectLessThanAlong<BBObject>::operator() (
    const BBObject& first, const BBObject& second)
{
    return 
	(first.GetBoundingBox ().*m_corner) ()[m_axis] < 
	(second.GetBoundingBox ().*m_corner) ()[m_axis];
}

template <typename BBObject>
double BBObjectLessThanAlong<BBObject>::operator() (
    const BBObject& x)
{
    return (x.GetBoundingBox ().*m_corner) ()[m_axis];
}


//======================================================================
// Template instantiations

// =====================================================================
// BBObjectLessThanAlong
template class BBObjectLessThanAlong<Body>;
template class BBObjectLessThanAlong<Foam>;
