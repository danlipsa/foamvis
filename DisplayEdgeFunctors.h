/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display an edge
 */

#ifndef __DISPLAY_EDGE_FUNCTORS_H__
#define __DISPLAY_EDGE_FUNCTORS_H__

#include "DisplayElement.h"

/**
 * Functor that displays an edge
 */
class DisplayTessellationOrPhysicalEdge : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayTessellationOrPhysicalEdge (const GLWidget& widget) : 
    DisplayElement (widget) {}
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const OrientedEdge* e)
    {
	float edgeSize = (e->GetEdge ()->IsPhysical ()) ? 
	    m_widget.GetPhysicalObjectsWidth () :
	    m_widget.GetTessellationObjectsWidth ();
	if (edgeSize != 0.0)
	{
	    Vertex* begin = e->GetBegin ();
	    Vertex* end = e->GetEnd ();
	    glLineWidth (edgeSize);
	    m_widget.qglColor (
		e->GetEdge()->IsPhysical () ? 
		m_widget.GetPhysicalObjectsColor () : 
		m_widget.GetTessellationObjectsColor () );
	    glBegin(GL_LINES);
	    glVertex3f(begin->x, begin->y, begin->z);
	    glVertex3f(end->x, end->y, end->z);
	    glEnd();
	}
    }
};


class DisplaySameEdges : public DisplayElement
{
public:
    DisplaySameEdges (const GLWidget& widget) : DisplayElement (widget) {}
    inline void operator() (const OrientedFace* f)
    {
	operator() (f->GetFace ());
    }
    
    void operator() (const Face* f)
    {
	glBegin (GL_POLYGON);
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), DisplaySameVertex());
	glEnd ();
    }
};


class DisplayDifferentEdges : public DisplayElement
{
public:
    DisplayDifferentEdges (const GLWidget& widget) : DisplayElement (widget) {}
    inline void operator() (const OrientedFace* f)
    {
	operator () (f->GetFace ());
    }
    void operator () (const Face* f)
    {
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), 
		  DisplayTessellationOrPhysicalEdge (m_widget));
    }
};


#endif //__DISPLAY_EDGE_FUNCTORS_H__
