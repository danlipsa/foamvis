/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display a vertex
 */

#include "DisplayVertexFunctors.h"

void DisplayAllVertices (const Edge& edge, double zPos)
{
    for (size_t i = 0; i < edge.PointCount (); i++)
    {
	G3D::Vector3 p = edge.GetPoint (i);
	glVertex (G3D::Vector3 (p.xy (), zPos));
    }
}
