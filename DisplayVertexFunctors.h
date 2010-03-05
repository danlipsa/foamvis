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

struct DisplayOriginalVertex
{
    void operator() (Vertex* v)
    {
	if (! v->IsDuplicate ())
	{
	    glVertex (*v);	
	}
    }
};

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
struct DisplaySameVertex
{
    void operator() (const OrientedEdge* e)
    {
	Vertex* p = e->GetBegin ();
	glVertex3f(p->x, p->y, p->z);
    }
};


class DisplayTessellationOrPhysicalVertex : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the vertex displayed
     */
    DisplayTessellationOrPhysicalVertex (const GLWidget& widget) : 
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
	    m_widget.GetPhysicalObjectsWidth () :
	    m_widget.GetTessellationObjectsWidth ();
	if (pointSize != 0.0)
	{
	    glPointSize (pointSize);
	    m_widget.qglColor (
		v->IsPhysical () ? 
		m_widget.GetPhysicalObjectsColor () : 
		m_widget.GetTessellationObjectsColor () );
	    glBegin(GL_POINTS);
	    glVertex3f(v->x, v->y, v->z);
	    glEnd();
	}
    }
};

class DisplayDifferentVertices : public DisplayElement
{
public:
    DisplayDifferentVertices (const GLWidget& widget) : 
	DisplayElement (widget) {}
    inline void operator() (const OrientedFace* f)
    {
	operator() (f->GetFace ());
    }
    void operator () (const Face* f)
    {
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (),
		  DisplayTessellationOrPhysicalVertex (m_widget));
    }
};



#endif //__DISPLAY_VERTEX_FUNCTORS_H__



