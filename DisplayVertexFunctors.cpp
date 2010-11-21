/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display a vertex
 */

#include "DisplayVertexFunctors.h"

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
