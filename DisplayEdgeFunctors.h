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
	glPushAttrib (GL_LIGHTING_BIT);
	const Vertex* begin = e->GetBegin ();
	const Vertex* end = e->GetEnd ();
	G3D::Vector3int16 domainIncrement = e->GetEndDomainIncrement ();
	setColor (m_widget.GetDomainIncrementColor (domainIncrement));
	if (domainIncrement != G3D::Vector3int16 (0, 0, 0))
	{
	    displayArrow (m_widget.GetQuadricObject (), 
			  m_widget.GetArrowBaseRadius (),
			  m_widget.GetArrowHeight (),
			  *begin, *end);
	}
	displayEdge (m_widget.GetQuadricObject (),
		     m_widget.GetEdgeRadius (), *begin, *end);
	glPopAttrib ();
    }

private:

    static void displayArrow (
	GLUquadricObj* quadric, float baseRadius, float height,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	G3D::Matrix3 rotation;
	edgeRotation (&rotation, begin, end);
	G3D::CoordinateFrame frame (rotation, (begin + end)/2);
	gluQuadricOrientation (quadric, GLU_OUTSIDE);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glMultMatrix (frame);
	gluCylinder (quadric, baseRadius, 0, height,
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (quadric, GLU_INSIDE);
	gluDisk (quadric, 0, baseRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
	glPopMatrix ();
    }

    static void displayEdge (
	GLUquadricObj* quadric, float edgeRadius,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	G3D::Matrix3 rotation;
	edgeRotation (&rotation, begin, end);
	G3D::CoordinateFrame frame (rotation, begin);
	gluQuadricOrientation (quadric, GLU_OUTSIDE);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glMultMatrix (frame);
	gluCylinder (quadric, edgeRadius, edgeRadius, (end - begin).length (),
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (quadric, GLU_INSIDE);
	gluDisk (quadric, 0, edgeRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
	glTranslatef (end.x, end.y, end.z);
	glPopMatrix ();
    }

    static void edgeRotation (
	G3D::Matrix3* rotation,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	using G3D::Vector3;
	Vector3 newZ = end - begin;
	if (newZ.isZero ())
	    return;
	newZ = newZ.unit ();
	Vector3 newX, newY;
	newZ.getTangents (newX, newY);
	rotation->setColumn (0, newX);
	rotation->setColumn (1, newY);
	rotation->setColumn (2, newZ);
    }

    static void setColor (const QColor& color)
    {
	GLfloat diffuseMaterial[] = 
	{
	    color.redF (), color.greenF (), color.blueF (), 1.0 
	};
	GLfloat ambientMaterial[] = 
	{
	    color.redF (), color.greenF (), color.blueF (), 1.0 
	};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMaterial);
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
