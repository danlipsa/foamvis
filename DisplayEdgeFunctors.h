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
#include "DebugStream.h"
#include "Edge.h"
#include "GLWidget.h"
#include "Foam.h"
#include "Utils.h"


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
	double edgeRadius,
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
	GLUquadricObj* quadric,
	double edgeRadius,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	static_cast<void> (quadric);
	static_cast<void> (edgeRadius);

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
	double baseRadius, double topRadius, double height,
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
	double, double, double,
	const G3D::Vector3& begin, const G3D::Vector3& end)
    {
	glLineWidth (2.0);
	glBegin(GL_LINES);
	glVertex(begin);
	glVertex((begin + end) / 2);
	glEnd();
    }
};


// Display one edge
// ======================================================================

template <typename displayEdge, typename displayArrow, bool showDuplicates>
class DisplayEdgeTorus : public DisplayElementFocus
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeTorus (const GLWidget& widget, FocusContext focus = FOCUS) : 
	DisplayElementFocus (widget, focus)
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
    void operator() (const boost::shared_ptr<OrientedEdge> oe) 
    {
	operator() (oe->GetEdge());
    }
    
    void operator() (const boost::shared_ptr<Edge>  e)
    {
	if (showDuplicates 
	    || e->GetDuplicateStatus () != ElementStatus::DUPLICATE)
	    display (e);
    }
protected:
    void display (const boost::shared_ptr<Edge>  e)
    {
	glPushAttrib (GL_LIGHTING_BIT);
	const Vertex* begin = e->GetBegin ().get ();
	const Vertex* end = e->GetEnd ().get ();
	G3D::Vector3int16 endLocation = e->GetEndTranslation ();
	glColor (m_glWidget.GetEndTranslationColor (endLocation));
	if (endLocation != Vector3int16Zero)
	    displayArrow() (
		m_glWidget.GetQuadricObject (), 
		m_glWidget.GetArrowBaseRadius (), m_glWidget.GetEdgeRadius (),
		m_glWidget.GetArrowHeight (),
		*begin, *end);
	displayEdge() (m_glWidget.GetQuadricObject (),
		       m_glWidget.GetEdgeRadius (), *begin, *end);
	glPopAttrib ();
    }
};


template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay = 
	  DisplayElement::TEST_DISPLAY_TESSELLATION>
class DisplayEdgeWithColor : public DisplayElementFocus
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeWithColor (const GLWidget& widget, 
			  FocusContext focus) : 
	DisplayElementFocus (widget, focus)
    {
    }
    void operator () (const boost::shared_ptr<Edge> edge) const
    {
	operator () (*edge);
    }

    void operator () (const Edge& edge) const
    {
	const Foam& foam = m_glWidget.GetCurrentFoam ();
	bool isPhysical = edge.IsPhysical (foam.GetSpaceDimension (),
					   foam.IsQuadratic ());
	if (isPhysical || 
	    (tesselationEdgesDisplay == TEST_DISPLAY_TESSELLATION &&
	     m_glWidget.IsEdgesTessellation () && 
	     m_focus == FOCUS))
	{
	    double alpha = 
		(m_focus == FOCUS ? 1.0 : m_glWidget.GetContextAlpha ());
	    Color::Enum color = edge.GetColor (Color::BLACK);
	    glColor (G3D::Color4 (Color::GetValue(color), alpha));
	    glBegin(GL_LINE_STRIP);
	    DisplayAllVertices (edge);
	    glEnd ();
	}
    }

    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const
    {
	operator () (oe->GetEdge ());
    }
};

class DisplayEdgeTorusClipped : public DisplayElementFocus
{
public:
    DisplayEdgeTorusClipped (const GLWidget& widget, FocusContext focus) : 
	DisplayElementFocus (widget, focus) 
    {
    }

    void operator () (const boost::shared_ptr<Edge>  edge) const
    {
	const OOBox& periods = m_glWidget.GetCurrentFoam ().GetOriginalDomain ();
	if (edge->IsClipped ())
	{
	    Color::Enum color = edge->GetColor (Color::BLACK);
	    glColor (Color::GetValue(color));
	    glBegin(GL_LINES);
	    for (size_t i = 0; i < edge->GetTorusClippedSize (periods); i++)
	    {
		glVertex(edge->GetTorusClippedBegin (i));
		glVertex (edge->GetTorusClippedEnd (i));
	    }
	    glEnd ();
	}
    }
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const
    {
	operator () (oe->GetEdge ());
    }
};


// Display all edges of a face
// ======================================================================

class DisplaySameEdges : public DisplayElementFocus
{
public:
    DisplaySameEdges (const GLWidget& widget, FocusContext focus = FOCUS) :
       DisplayElementFocus (widget, focus)
    {
    }
    inline void operator() (const boost::shared_ptr<OrientedFace>  f)
    {
       operator() (f->GetFace ());
    }

    void operator() (const boost::shared_ptr<Face>& f)
    {
       glBegin (GL_POLYGON);
       const vector<boost::shared_ptr<OrientedEdge> >& v =
           f->GetOrientedEdges ();
       for_each (v.begin (), v.end (), DisplayAllButLastVertices ());
       if (! f->IsClosed ())
           glVertex (*v[v.size () - 1]->GetEnd ());
       glEnd ();
    }
};


/**
 * Functor that displays an edge
 */
class DisplaySameTriangles : public DisplayElementFocus
{
public:
    DisplaySameTriangles (const GLWidget& widget, FocusContext focus = FOCUS) : 
	DisplayElementFocus (widget, focus) 
    {
    }
    void operator() (const boost::shared_ptr<OrientedFace>  f)
    {
	operator() (f->GetFace ());
    }
    
    void operator() (const boost::shared_ptr<Face>& f)
    {
	const vector<boost::shared_ptr<OrientedEdge> >& orientedEdges = 
	    f->GetOrientedEdges ();
	glBegin (GL_TRIANGLES);
	if (f->IsTriangle ())
	    for_each (orientedEdges.begin (), orientedEdges.end (),
		      DisplayBeginVertex());
	else
	{
	    DisplayTriangle dt (f->GetCenter ());
	    for_each (orientedEdges.begin (), orientedEdges.end (),
		      boost::bind (dt, _1));
	    if (! f->IsClosed ())
		dt(*orientedEdges[orientedEdges.size () - 1]->GetEnd (),
		   *orientedEdges[0]->GetBegin ());
	}
	glEnd ();
    }
};


template<typename displayEdge>
class DisplayEdges : public DisplayElementFocus
{
public:
    DisplayEdges (const GLWidget& widget, FocusContext focus) : 
	DisplayElementFocus (widget, focus)
    {
    }

    void operator() (const boost::shared_ptr<OrientedFace>  f)
    {
	operator() (f->GetFace ());
    }
    void operator () (const boost::shared_ptr<Face>  f)
    {
	const vector< boost::shared_ptr<OrientedEdge> >& v = 
	    f->GetOrientedEdges ();
	displayEdge display(m_glWidget, m_focus);
	for (size_t i = 0; i < v.size (); i++)
	{
	    boost::shared_ptr<OrientedEdge> oe = v[i];
	    size_t displayedEdgeIndex = m_glWidget.GetDisplayedEdgeIndex ();
	    if (m_glWidget.IsDisplayedEdge (i))
	    {
		display (oe);
		if (i == displayedEdgeIndex)
		    cdbg << "edge " << i << ": " << *oe;
	    }
	}
    }
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
