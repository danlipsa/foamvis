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

    ApproximationEdge (begin, end, endLocation, id, duplicateStatus),
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
    const OOBox& periods,
    const G3D::Vector3& newBegin, VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = periods.GetTranslation (
	GetBegin ()->GetVector (), newBegin);
    boost::shared_ptr<QuadraticEdge> duplicate = 
	boost::static_pointer_cast<QuadraticEdge> (
	    Edge::createDuplicate (periods, newBegin, vertexSet));
    boost::shared_ptr<Vertex> middleDuplicate = GetMiddle ()->GetDuplicate (
	periods, translation, vertexSet);
    duplicate->setMiddle (middleDuplicate);
    duplicate->cachePoints ();
    return duplicate;
}

double QuadraticEdge::quadratic (double t, size_t i) const
{
    return
	GetBegin ()->GetVector ()[i] * (1 - t) * (2 - t) / 2 +
	GetMiddle ()->GetVector ()[i] * t * (2 - t) +
	GetEnd ()->GetVector ()[i] * t * (t - 1) / 2;
}

G3D::Vector3 QuadraticEdge::quadratic (double t) const
{
    G3D::Vector3 result;
    for (size_t i = 0; i < 3; i++)
	result[i] = quadratic (t, i);
    return result;
}

G3D::Vector3 QuadraticEdge::computePoint (size_t i) const
{
    return quadratic ( static_cast<double>(i) * 2 / (GetPointCount () - 1));
}
