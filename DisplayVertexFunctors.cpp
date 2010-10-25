/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display a vertex
 */

#include "DisplayVertexFunctors.h"

void DisplayAllVertices (const Edge& edge)
{
    for (size_t i = 0; i < edge.PointCount (); i++)
	glVertex (edge.GetPoint (i));
}
