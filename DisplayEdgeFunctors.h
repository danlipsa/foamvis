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
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "GLWidget.h"
#include "Foam.h"
#include "Utils.h"
#include "OpenGLUtils.h"
#include "OrientedEdge.h"

class DisplayEdge
{
public:
    DisplayEdge (GLUquadricObj* quadric = 0, double edgeRadius = 0) :
	m_quadric (quadric), m_edgeRadius (edgeRadius)
    {
    }

    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);

protected:
    GLUquadricObj* m_quadric;
    double m_edgeRadius;
};

class DisplayEdgeTube : public DisplayEdge
{
public:
    DisplayEdgeTube (GLUquadricObj* quadric, double edgeRadius) :
	DisplayEdge (quadric, edgeRadius)
    {
    }
    
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayArrow
{
public:
    enum Position
    {
	BASE_MIDDLE,
	TOP_END
    };

public:
    DisplayArrow (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 0, double height = 0,
	Position position = BASE_MIDDLE) :
	m_quadric (quadric), m_baseRadius (baseRadius), m_topRadius(topRadius),
	m_height (height), m_position (position)
    {
    }

    void operator () (const G3D::Vector3& begin, const G3D::Vector3& end);
protected:
    GLUquadricObj* m_quadric;
    double m_baseRadius;
    double m_topRadius;
    double m_height;
    Position m_position;
};

class DisplayArrowTube : public DisplayArrow
{
public:
    DisplayArrowTube (
	GLUquadricObj* quadric,
	double baseRadius, double topRadius, double height, 
	Position position = BASE_MIDDLE) :
	DisplayArrow (quadric, baseRadius, topRadius, height, position)
    {
    }

    void operator () (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayOrientedEdge : public DisplayArrow
{
public:
    DisplayOrientedEdge (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 0, double height = 0,
	Position position = BASE_MIDDLE) :
	DisplayArrow (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayOrientedEdgeTube : public DisplayArrow
{
public:
    DisplayOrientedEdgeTube (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 0, double height = 0,
	Position position = BASE_MIDDLE) :
	DisplayArrow (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

// Display one edge
// ======================================================================

template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
class DisplayEdgeTorus : public DisplayElementFocus
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    DisplayEdgeTorus (const GLWidget& widget, FocusContext focus = FOCUS, 
		      bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos),
	m_displayEdge (m_glWidget.GetQuadricObject (), 
		       m_glWidget.GetEdgeRadius ()),
	m_displayArrow (
	    m_glWidget.GetQuadricObject (), m_glWidget.GetArrowBaseRadius (),
	    m_glWidget.GetEdgeRadius (), m_glWidget.GetArrowHeight ())
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
	glPushAttrib (GL_CURRENT_BIT);
	const Vertex* begin = e->GetBegin ().get ();
	const Vertex* end = e->GetEnd ().get ();
	G3D::Vector3int16 endLocation = e->GetEndTranslation ();
	glColor (m_glWidget.GetEndTranslationColor (endLocation));

	if (endLocation != Vector3int16Zero)
	    m_displayArrow(*begin, *end);
	m_displayEdge(*begin, *end);
	glPopAttrib ();
    }
private:
    DisplayEdge m_displayEdge;
    DisplayArrow m_displayArrow;
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
			  FocusContext focus,
			  bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }
    void operator () (const boost::shared_ptr<Edge> edge) const
    {
	operator () (*edge);
    }

    void operator () (const Edge& edge) const
    {
	const Foam& foam = m_glWidget.GetCurrentFoam ();
	bool isPhysical = edge.IsPhysical (foam.GetDimension (),
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
	    DisplayAllVertices (edge, m_useZPos, m_zPos);
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


/**
 * Used for displaying edges clipped at the torus original domain boudary.
 * @todo Use circular arcs in this view as well.
 */
class DisplayEdgeTorusClipped : public DisplayElementFocus
{
public:
    DisplayEdgeTorusClipped (const GLWidget& widget, FocusContext focus, 
			     bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }

    void operator () (const boost::shared_ptr<Edge>& edge) const;
    inline void operator() (const boost::shared_ptr<OrientedEdge>& oe) const
    {
	operator () (oe->GetEdge ());
    }
};


// Display all edges of a face
// ======================================================================

class DisplaySameEdges : public DisplayElementFocus
{
public:
    DisplaySameEdges (const GLWidget& widget, FocusContext focus = FOCUS,
		      bool useZPos = false, double zPos = 0) :
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }
    inline void operator() (const boost::shared_ptr<OrientedFace>&  of)
    {
	operator() (of->GetFace ());
    }

    void operator() (const boost::shared_ptr<Face>& f);
};


/**
 * Functor that displays an edge
 */
class DisplaySameTriangles : public DisplayElementFocus
{
public:
    DisplaySameTriangles (const GLWidget& widget, FocusContext focus = FOCUS,
			  bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }
    void operator() (const boost::shared_ptr<OrientedFace>& of)
    {
	operator() (of->GetFace ());
    }
    void operator() (const boost::shared_ptr<Face>& f);
};


template<typename displayEdge>
class DisplayEdges : public DisplayElementFocus
{
public:
    DisplayEdges (const GLWidget& widget, FocusContext focus, 
		  bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos)
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
	displayEdge display(m_glWidget, m_focus, m_useZPos, m_zPos);
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
