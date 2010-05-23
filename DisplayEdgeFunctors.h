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
#include "Edge.h"
#include "GLWidget.h"
#include "Foam.h"

void edgeRotation (
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

struct DisplayEdgeTube
{
    void operator() (
	GLUquadricObj* quadric,
	float edgeRadius,
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
};

struct DisplayEdge
{
    void operator() (
	GLUquadricObj*,
	float,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	glLineWidth (1.0);
	glBegin(GL_LINES);
	glVertex(begin);
	glVertex(end);
	glEnd();
    }
};

struct DisplayArrowTube
{
    void operator () (
	GLUquadricObj* quadric,
	float baseRadius, float topRadius, float height,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	G3D::Matrix3 rotation;
	edgeRotation (&rotation, begin, end);
	G3D::CoordinateFrame frame (rotation, (begin + end)/2);
	gluQuadricOrientation (quadric, GLU_OUTSIDE);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glMultMatrix (frame);
	gluCylinder (quadric, baseRadius, topRadius, height,
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (quadric, GLU_INSIDE);
	gluDisk (quadric, 0, baseRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
	glPopMatrix ();
    }
};

struct DisplayArrow
{
    void operator () (
	GLUquadricObj*,
	float, float, float,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	glLineWidth (2.0);
	glBegin(GL_LINES);
	glVertex(begin);
	glVertex((begin + end) / 2);
	glEnd();
    }
};


template <typename displayEdge, typename displayArrow, bool showDuplicates>
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
    void operator () (const OrientedEdge& oe)
    {
	operator () (oe.GetEdge ());
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
	if (showDuplicates || e->GetStatus () != ElementStatus::DUPLICATE)
	    display (e);
    }
protected:
    void display (const Edge* e)
    {
	glPushAttrib (GL_LIGHTING_BIT);
	const Vertex* begin = e->GetBegin ();
	const Vertex* end = e->GetEnd ();
	G3D::Vector3int16 endLocation = e->GetEndTranslation ();
	m_widget.qglColor (m_widget.GetEndTranslationColor (endLocation));
	if (endLocation != G3D::Vector3int16 (0, 0, 0))
	    displayArrow() (
		m_widget.GetQuadricObject (), 
		m_widget.GetArrowBaseRadius (), m_widget.GetEdgeRadius (),
		m_widget.GetArrowHeight (),
		*begin, *end);
	displayEdge() (m_widget.GetQuadricObject (),
		       m_widget.GetEdgeRadius (), *begin, *end);
	glPopAttrib ();
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
    void operator () (const OrientedEdge& oe)
    {
	operator () (&oe);
    }
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const OrientedEdge* e)
    {
	float edgeSize = (e->GetEdge ()->IsPhysical ()) ? 
	    m_widget.GetPhysicalEdgeWidth () :
	    m_widget.GetTessellationEdgeWidth ();
	if (edgeSize != 0.0)
	{
	    Vertex* begin = e->GetBegin ();
	    Vertex* end = e->GetEnd ();
	    glLineWidth (edgeSize);
	    m_widget.qglColor (
		e->GetEdge()->IsPhysical () ? 
		m_widget.GetPhysicalEdgeColor () : 
		m_widget.GetTessellationEdgeColor () );
	    glBegin(GL_LINES);
	    glVertex3f(begin->x, begin->y, begin->z);
	    glVertex3f(end->x, end->y, end->z);
	    glEnd();
	}
    }
};


class DisplayEdgeWithColor : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeWithColor (const GLWidget& widget) : 
    DisplayElement (widget) {}
    void operator () (const Edge* edge) const
    {
	Color::Name color = edge->GetColor (Color::BLACK);
	glColor4fv (Color::GetValue(color));
	G3D::Vector3* b = edge->GetBegin ();
	G3D::Vector3* e = edge->GetEnd ();
	glBegin(GL_LINES);
	glVertex(*b);
	glVertex (*e);
	glEnd ();
    }
    void operator () (const OrientedEdge& oe) const
    {
	operator () (&oe);
    }
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const OrientedEdge* oe) const
    {
	operator () (oe->GetEdge ());
    }
};

class DisplayEdgeTorusClipped : public DisplayElement
{
public:
    DisplayEdgeTorusClipped (const GLWidget& widget) : 
	DisplayElement (widget) {}
    void operator () (const Edge* edge) const
    {
	const OOBox& periods = m_widget.GetCurrentFoam ().GetPeriods ();
	if (edge->IsClipped ())
	{
	    Color::Name color = edge->GetColor (Color::BLACK);
	    glColor4fv (Color::GetValue(color));
	    glBegin(GL_LINES);
	    for (size_t i = 0; i < edge->GetTorusClippedSize (periods); i++)
	    {
		glVertex(edge->GetTorusClippedBegin (i));
		glVertex (edge->GetTorusClippedEnd (i));
	    }
	    glEnd ();
	}
    }
    void operator() (const OrientedEdge* oe) const
    {
	operator () (oe->GetEdge ());
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
	for_each (v.begin (), v.end (), DisplayBeginVertex());
	if (! f->IsClosed ())
	    DisplayEdgeVertices () (v[v.size () - 1]);
	glEnd ();
    }
};


template<typename displayEdge>
class DisplayEdges : public DisplayElement
{
public:
    DisplayEdges (const GLWidget& widget) : DisplayElement (widget) {}
    void operator() (const OrientedFace* f)
    {
	operator() (f->GetFace ());
    }
    void operator () (const Face* f)
    {
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	displayEdge display(m_widget);
	for (size_t i = 0; i < v.size (); i++)
	{
	    OrientedEdge* oe = v[i];
	    size_t displayedEdge = m_widget.GetDisplayedEdgeIndex ();
	    displayEdge display(m_widget);
	    if (displayedEdge == GLWidget::DISPLAY_ALL || displayedEdge == i)
	    {
		display (oe);
		if (i == displayedEdge)
		    cdbg << "edge " << i << ": " << *oe << endl;
	    }
	}
    }
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
