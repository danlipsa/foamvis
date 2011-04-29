/**
 * @file   ApproximationEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ApproximationEdge class
 */

#include "Debug.h"
#include "ApproximationEdge.h"

ApproximationEdge::ApproximationEdge (
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end,
    const G3D::Vector3int16& endLocation, 
    size_t id,
    ElementStatus::Enum duplicateStatus) :

    Edge (begin, end, endLocation, id, duplicateStatus)
{
}

ApproximationEdge::ApproximationEdge (
    const ApproximationEdge& approximationEdge) :

    Edge (approximationEdge)
{
}

void ApproximationEdge::cachePoints ()
{
    for (size_t i = 0; i < m_points.size (); ++i)
	m_points[i] = computePoint (i);
}

size_t ApproximationEdge::GetPointCount () const
{
    return POINT_COUNT;
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
