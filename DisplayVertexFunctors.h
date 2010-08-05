/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display a vertex
 */

#ifndef __DISPLAY_VERTEX_FUNCTORS_H__
#define __DISPLAY_VERTEX_FUNCTORS_H__

#include "DisplayElement.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "GLWidget.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "Vertex.h"

struct DisplayOriginalVertex
{
    void operator() (const boost::shared_ptr<Vertex>& v)
    {
	if (v->GetDuplicateStatus () != ElementStatus::DUPLICATE)
	{
	    glVertex (*v);	
	}
    }
};

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
struct DisplayBeginVertex
{
    DisplayBeginVertex () {}
    DisplayBeginVertex (const GLWidget&) {}
    void operator() (const boost::shared_ptr<OrientedEdge> e)
    {
	glVertex (*e->GetBegin ());
	/*
	for (size_t i = 0; i < e->PointCount () - 1; ++i)
	    glVertex(e->GetPoint (i));
	*/
    }
};

struct DisplayEdgeVertices
{
    DisplayEdgeVertices () {}
    DisplayEdgeVertices (const GLWidget&) {}
    void operator() (const boost::shared_ptr<OrientedEdge> edge)
    {
	const Vertex& b = *edge->GetBegin ();
	glVertex (b);
	const Vertex& e = *edge->GetEnd ();
	glVertex (e);
    }
};


#endif //__DISPLAY_VERTEX_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
