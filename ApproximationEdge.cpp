/**
 * @file   ApproximationEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ApproximationEdge class
 */

#include "ApproximationEdge.h"
#include "Debug.h"
#include "OOBox.h"
#include "Vertex.h"

ApproximationEdge::ApproximationEdge (
    size_t pointCount,
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end,
    const G3D::Vector3int16& endLocation, 
    size_t id, Edge::Type type,
    ElementStatus::Enum duplicateStatus) :

    Edge (begin, end, endLocation, id, type, duplicateStatus),
    m_points (pointCount)
{
}

ApproximationEdge::ApproximationEdge (const ApproximationEdge& other) :
    Edge (other)
{
    m_points.resize (other.m_points.size ());
    copy (other.m_points.begin (), other.m_points.end (), m_points.begin ());
}

size_t ApproximationEdge::GetPointCount () const
{
    return m_points.size ();
}

G3D::Vector3 ApproximationEdge::GetPoint (size_t i) const
{
    return m_points[i];
}

boost::shared_ptr<Edge> ApproximationEdge::createDuplicate (
    const OOBox& originalDomain,
    const G3D::Vector3& newBegin, VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = originalDomain.GetTranslation (
	GetBeginVector (), newBegin);
    boost::shared_ptr<ApproximationEdge> duplicate = 
	boost::static_pointer_cast<ApproximationEdge> (
	    Edge::createDuplicate (originalDomain, newBegin, vertexSet));
    for (size_t i = 0; i < m_points.size (); ++i)
    {
	G3D::Vector3& point = duplicate->m_points[i];
	point = originalDomain.TorusTranslate (point, translation);
    }
    return duplicate;
}
