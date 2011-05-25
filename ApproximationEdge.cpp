/**
 * @file   ApproximationEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ApproximationEdge class
 */

#include "Debug.h"
#include "ApproximationEdge.h"
#include "Vertex.h"

ApproximationEdge::ApproximationEdge (
    size_t pointCount,
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end,
    const G3D::Vector3int16& endLocation, 
    size_t id,
    ElementStatus::Enum duplicateStatus) :

    Edge (begin, end, endLocation, id, duplicateStatus),
    m_points (pointCount)
{
}

ApproximationEdge::ApproximationEdge (const ApproximationEdge& other) :

    Edge (other)
{
    m_points.resize (other.m_points.size ());
    copy (other.m_points.begin (), other.m_points.end (), m_points.begin ());
}

void ApproximationEdge::cachePoints ()
{
    m_points[0] = GetBegin ()->GetVector ();
    m_points[m_points.size () - 1] = GetEnd ()->GetVector ();
    for (size_t i = 1; i < m_points.size () - 1; ++i)
	m_points[i] = computePoint (i);
}

size_t ApproximationEdge::GetPointCount () const
{
    return m_points.size ();
}

G3D::Vector3 ApproximationEdge::GetPoint (size_t i) const
{
    return m_points[i];
}

void ApproximationEdge::SetEnd(boost::shared_ptr<Vertex> end) 
{
    Edge::SetEnd (end);
    cachePoints ();
}
