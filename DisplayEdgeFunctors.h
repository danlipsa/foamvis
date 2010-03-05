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

class DisplayEdgeTorus : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeTorus (const GLWidget& widget) : DisplayElement (widget) 
    {
    }
    ~DisplayEdgeTorus() 
    {
    }
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const OrientedEdge* oe) 
    {
	operator() (oe->GetEdge());
    }
    
    void operator() (const Edge* e)
    {
	display (e);
    }
protected:
    void display (const Edge* e)
    {
	const Vertex* begin = e->GetBegin ();
	const Vertex* end = e->GetEnd ();
	G3D::Vector3int16 domainIncrement = e->GetEndDomainIncrement ();
	m_widget.qglColor (m_widget.GetDomainIncrementColor (domainIncrement));
	if (domainIncrement != G3D::Vector3int16 (0, 0, 0))
	    displayArrow (m_widget.GetQuadricObject (), *begin, *end);
	glBegin(GL_LINES);
	glVertex3f(begin->x, begin->y, begin->z);
	glVertex3f(end->x, end->y, end->z);
	glEnd();
    }

private:
    static void displayArrow (
	GLUquadricObj* quadric,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	using G3D::Vector3;
	Vector3 newZ = end - begin;
	if (newZ.isZero ())
	    return;
	newZ = newZ.unit ();
	Vector3 newX = aNormal (newZ);
	Vector3 newY = newZ.cross (newX);
	G3D::Matrix3 rotation;
	rotation.setColumn (0, newX);
	rotation.setColumn (1, newY);
	rotation.setColumn (2, newZ);
	G3D::CoordinateFrame frame (rotation, (begin + end)/2);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);	
	gluQuadricDrawStyle (quadric, GLU_FILL);
	gluQuadricNormals (quadric, GLU_SMOOTH);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glMultMatrix (frame);
	gluCylinder (quadric, .05, 0, .1, 20, 20);
	glPopMatrix ();
    }

};

class DisplayOriginalEdgeTorus : public DisplayEdgeTorus
{
public:
    DisplayOriginalEdgeTorus (const GLWidget& widget) : 
    DisplayEdgeTorus (widget) {}
    void operator () (const Edge* e)
    {
	if (! e->IsDuplicate ())
	    display (e);
    }
};


class DisplayEdgeTessellationOrPhysical : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeTessellationOrPhysical (const GLWidget& widget) : 
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



/**
 * Functor that displays an edge
 */
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


template<typename displayEdge>
class DisplayEdges : public DisplayElement
{
public:
    DisplayEdges (const GLWidget& widget) : DisplayElement (widget) {}
    inline void operator() (const OrientedFace* f)
    {
	operator () (f->GetFace ());
    }
    void operator () (const Face* f)
    {
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), displayEdge (m_widget));
    }
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__
