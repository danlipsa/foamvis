/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display an edge
 */

#ifndef __DISPLAY_EDGE_FUNCTORS_H__
#define __DISPLAY_EDGE_FUNCTORS_H__

class displaySameEdges
{
public:
    displaySameEdges (const GLWidget& glWidget) : m_glWidget (glWidget) {}
    inline void operator() (OrientedFace* f)
    {
	operator() (f->GetFace ());
    }
    
    void operator() (Face* f)
    {
	glBegin (GL_POLYGON);
	vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), displaySameVertex());
	glEnd ();
    }

private:
    const GLWidget& m_glWidget;
};


/**
 * Functor that displays an edge
 */
class displayTessellationOrPhysicalEdge
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    displayTessellationOrPhysicalEdge (const GLWidget& widget) : 
	m_widget (widget) 
    {
    }
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (OrientedEdge* e)
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
protected:
    /**
     * Where should be the vertex displayed
     */
    const GLWidget& m_widget;
};


class displayDifferentEdges
{
public:
    displayDifferentEdges (const GLWidget& glWidget) : m_glWidget (glWidget) {}
    inline void operator() (OrientedFace* f)
    {
	operator () (f->GetFace ());
    }
    void operator () (Face* f)
    {
	vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), 
		  displayTessellationOrPhysicalEdge (m_glWidget));
    }
private:
    const GLWidget& m_glWidget;
};



#endif //__DISPLAY_EDGE_FUNCTORS_H__
