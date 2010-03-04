/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display a vertex
 */

#ifndef __DISPLAY_VERTEX_FUNCTORS_H__
#define __DISPLAY_VERTEX_FUNCTORS_H__

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
struct displaySameVertex
{
    void operator() (OrientedEdge* e)
    {
	Vertex* p = e->GetBegin ();
	glVertex3f(p->x, p->y, p->z);
    }
};


/**
 * Functor that displays a vertex
 */
class displayDifferentVertex
{
public:
    /**
     * Constructor
     * @param widget Where should be the vertex displayed
     */
    displayDifferentVertex (const GLWidget& glWidget) : 
	m_glWidget (glWidget) 
    {
    }
    /**
     * Functor that displays a vertex
     * @param v the vertex to be displayed
     */
    void operator() (OrientedEdge* oe)
    {
	Vertex* v = oe->GetBegin ();
	float pointSize = (v->IsPhysical ()) ? 
	    m_glWidget.GetPhysicalObjectsWidth () :
	    m_glWidget.GetTessellationObjectsWidth ();
	if (pointSize != 0.0)
	{
	    glPointSize (pointSize);
	    m_glWidget.qglColor (
		v->IsPhysical () ? 
		m_glWidget.GetPhysicalObjectsColor () : 
		m_glWidget.GetTessellationObjectsColor () );
	    glBegin(GL_POINTS);
	    glVertex3f(v->x, v->y, v->z);
	    glEnd();
	}
    }
private:
    /**
     * Where should be the vertex displayed
     */
    const GLWidget& m_glWidget;
};

class displayDifferentVertices
{
public:
    displayDifferentVertices (const GLWidget& glWidget) : 
	m_glWidget (glWidget) {}
    inline void operator() (OrientedFace* f)
    {
	operator() (f->GetFace ());
    }
    void operator () (Face* f)
    {
	vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), displayDifferentVertex (m_glWidget));
    }
private:
    const GLWidget& m_glWidget;
};



#endif //__DISPLAY_VERTEX_FUNCTORS_H__



