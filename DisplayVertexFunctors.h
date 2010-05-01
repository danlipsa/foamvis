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
#include "Face.h"
#include "GLWidget.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "Vertex.h"

struct DisplayOriginalVertex
{
    void operator() (Vertex* v)
    {
	if (v->GetStatus () != ElementStatus::DUPLICATE)
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
    void operator() (const OrientedEdge* e)
    {
	Vertex* p = e->GetBegin ();
	glVertex(*p);
    }
};

struct DisplayEdgeVertices
{
    DisplayEdgeVertices () {}
    DisplayEdgeVertices (const GLWidget&) {}
    void operator() (const OrientedEdge* edge)
    {
	Vertex& b = *edge->GetBegin ();
	glVertex (b);
	Vertex& e = *edge->GetEnd ();
	glVertex (e);
    }
};




class DisplayPhysicalVertex : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the vertex displayed
     */
    DisplayPhysicalVertex (const GLWidget& widget) : 
    DisplayElement (widget) 
    {
    }
    /**
     * Functor that displays a vertex
     * @param v the vertex to be displayed
     */
    void operator() (const OrientedEdge* oe)
    {
	Vertex* v = oe->GetBegin ();
	float pointSize = (v->IsPhysical ()) ? 
	    m_widget.GetPhysicalVertexSize () :
	    m_widget.GetTessellationVertexSize ();
	if (pointSize != 0.0)
	{
	    glPointSize (pointSize);
	    m_widget.qglColor (
		v->IsPhysical () ? 
		m_widget.GetPhysicalVertexColor () : 
		m_widget.GetTessellationVertexColor () );
	    glBegin(GL_POINTS);
	    glVertex3f(v->x, v->y, v->z);
	    glEnd();
	}
    }
};


#endif //__DISPLAY_VERTEX_FUNCTORS_H__



