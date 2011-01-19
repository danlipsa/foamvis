/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display a vertex
 */

#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "OrientedEdge.h"
#include "Vertex.h"

void DisplayEdgeVertices (const Edge& edge, bool useZPos, double zPos)
{
    for (size_t i = 0; i < edge.PointCount (); i++)
    {
	G3D::Vector3 p = edge.GetPoint (i);
	glVertex (useZPos ? G3D::Vector3 (p.xy (), zPos) : p);
    }
}

void  DisplayAllButLastVertices (const boost::shared_ptr<OrientedEdge> e)
{
    for (size_t i = 0; i < e->PointCount () - 1; ++i)
	glVertex(e->GetPoint (i));
}

// DisplayOriginalVertex
// ======================================================================

void DisplayOriginalVertex::operator() (const boost::shared_ptr<Vertex>& v)
{
    if (v->GetDuplicateStatus () != ElementStatus::DUPLICATE)
    {
	glVertex (*v);	
    }
}

// DisplayBeginVertex
// ======================================================================

void DisplayBeginVertex::operator() (const boost::shared_ptr<OrientedEdge> e)
{
    glVertex (*e->GetBegin ());
    //for (size_t i = 0; i < e->PointCount (); ++i)
    //glVertex(e->GetPoint (i));
	
}

// DisplayTriangle
// ======================================================================

void DisplayTriangle::operator() (const boost::shared_ptr<OrientedEdge> e) const
{
    operator () (*e->GetBegin (), *e->GetEnd ());
}

void DisplayTriangle::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end) const
{
    glVertex (m_center);
    glVertex (begin);
    glVertex (end);
}
