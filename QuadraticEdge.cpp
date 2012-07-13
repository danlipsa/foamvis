/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */

#include "Debug.h"
#include "OOBox.h"
#include "QuadraticEdge.h"
#include "Vertex.h"

QuadraticEdge::QuadraticEdge (
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end,
    const boost::shared_ptr<Vertex>& middle,
    const G3D::Vector3int16& endLocation, 
    size_t id,
    ElementStatus::Enum duplicateStatus) :

    ApproximationEdge (5, begin, end, endLocation, id, QUADRATIC_EDGE, 
		       duplicateStatus),
    m_middle (middle)
{
    cachePoints ();
}

QuadraticEdge::QuadraticEdge (const QuadraticEdge& quadraticEdge) :
    ApproximationEdge (quadraticEdge),
    m_middle (quadraticEdge.m_middle)
{
    cachePoints ();
}

boost::shared_ptr<Edge> QuadraticEdge::Clone () const
{
    return boost::shared_ptr<Edge> (new QuadraticEdge(*this));
}

boost::shared_ptr<Edge> QuadraticEdge::createDuplicate (
    const OOBox& originalDomain,
    const G3D::Vector3& newBegin, VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = originalDomain.GetTranslation (
	GetBeginVector (), newBegin);
    boost::shared_ptr<QuadraticEdge> duplicate = 
	boost::static_pointer_cast<QuadraticEdge> (
	    Edge::createDuplicate (originalDomain, newBegin, vertexSet));
    boost::shared_ptr<Vertex> middleDuplicate = GetMiddle ().GetDuplicate (
	originalDomain, translation, vertexSet);
    duplicate->setMiddle (middleDuplicate);
    duplicate->cachePoints ();
    return duplicate;
}

double QuadraticEdge::quadratic (double t, size_t axis) const
{
    return
	GetBeginVector ()[axis] * (1 - t) * (2 - t) / 2 +
	GetMiddle ().GetVector ()[axis] * t * (2 - t) +
	GetEndVector ()[axis] * t * (t - 1) / 2;
}

G3D::Vector3 QuadraticEdge::quadratic (double t) const
{
    G3D::Vector3 result;
    for (size_t axis = 0; axis < 3; ++axis)
	result[axis] = quadratic (t, axis);
    return result;
}

G3D::Vector3 QuadraticEdge::calculatePoint (size_t i) const
{
    return quadratic (static_cast<double>(i) * 2 / (GetPointCount () - 1));
}

void QuadraticEdge::cachePoints ()
{
    m_points[0] = GetBeginVector ();
    m_points[m_points.size () - 1] = GetEndVector ();
    for (size_t i = 1; i < m_points.size () - 1; ++i)
	m_points[i] = calculatePoint (i);
}

void QuadraticEdge::SetEnd(boost::shared_ptr<Vertex> end) 
{
    Edge::SetEnd (end);
    cachePoints ();
}

G3D::Vector3 QuadraticEdge::GetMiddleVector () const
{
    return GetMiddle ().GetVector ();
}
