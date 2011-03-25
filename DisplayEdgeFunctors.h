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
class Disk;
class Edge;
class Face;
class OrientedEdge;
class OrientedFace;


struct Segment
{
    Segment () :
	m_perpendicularEnd (SegmentPerpendicularEnd::COUNT),
	m_context (false)
    {
    }

    Segment (SegmentPerpendicularEnd::Enum perpendicularEnd, 
	     const G3D::Vector3& beforeBegin,
	     const G3D::Vector3& begin,
	     const G3D::Vector3& end,
	     const G3D::Vector3& afterEnd, bool context) :

	m_perpendicularEnd (perpendicularEnd),
	m_beforeBegin (beforeBegin),
	m_begin (begin),
	m_end (end),
	m_afterEnd (afterEnd),
	m_context (context)
    {
    }

    SegmentPerpendicularEnd::Enum m_perpendicularEnd;
    G3D::Vector3 m_beforeBegin;
    G3D::Vector3 m_begin;
    G3D::Vector3 m_end;
    G3D::Vector3 m_afterEnd;
    bool m_context;
};


class DisplayEdge
{
public:
    DisplayEdge () :
	m_quadric (0), m_radius (1.0), m_contextRadius (1.0)
    {
    }
    

    DisplayEdge (GLUquadricObj* quadric, double edgeRadius, 
		 double contextEdgeRadius = 2.0) :
	
	m_quadric (quadric), m_radius (edgeRadius), 
	m_contextRadius (contextEdgeRadius)
    {
    }

    void operator () (
	const G3D::Vector3& begin, const G3D::Vector3& end, 
	bool context = false);
    void operator () (const Segment& segment)
    {
	operator () (segment.m_begin, segment.m_end, segment.m_context);
    }

protected:
    GLUquadricObj* m_quadric;
    double m_radius;
    double m_contextRadius;
};

class DisplayEdgeQuadric : public DisplayEdge
{
public:
    DisplayEdgeQuadric () : DisplayEdge ()
    {
    }

    DisplayEdgeQuadric (
	GLUquadricObj* quadric, double edgeRadius, double contextRadius = 2.0) :
	DisplayEdge (quadric, edgeRadius, contextRadius)
    {
    }
    
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
    void operator () (const Segment& segment)
    {
	operator () (segment.m_begin, segment.m_end);
    }
};

class DisplayEdgeTube : public DisplayEdge
{
public:
    DisplayEdgeTube () : DisplayEdge ()
    {
    }

    DisplayEdgeTube (GLUquadricObj* quadric, double edgeRadius, 
		     double contextRadius = 2.0) :
	DisplayEdge (quadric, edgeRadius, contextRadius)
    {
    }
    
    void operator() (const Segment& segment);
private:
    void displayTube (const Disk& begin, const Disk& end) const;
    Disk perpendicularDisk (
	const G3D::Vector3& beginEdge, const G3D::Vector3& endEdge, 
	const G3D::Vector3& origin) const;
    Disk angledDisk (
	const G3D::Vector3& beforeP, const G3D::Vector3& p, 
	const G3D::Vector3& afterP, const G3D::Vector3& origin) const;
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
	m_quadric (quadric),
	m_baseRadius (baseRadius),
	m_topRadius(topRadius),
	m_height (height),
	m_position (position)
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

class DisplayArrowQuadric : public DisplayArrow
{
public:
    DisplayArrowQuadric (
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
	double baseRadius = 0, double topRadius = 1.0, double height = 0,
	Position position = BASE_MIDDLE) :
	DisplayArrow (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayOrientedEdgeQuadric : public DisplayArrow
{
public:
    DisplayOrientedEdgeQuadric (
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
		      bool useZPos = false, double zPos = 0);

    void operator () (const OrientedEdge& oe);

    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedEdge> oe);
    
    void operator() (const boost::shared_ptr<Edge>  e);

protected:
    void display (const boost::shared_ptr<Edge>  e);

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
			  bool useZPos = false, double zPos = 0);

    void operator () (const boost::shared_ptr<Edge> edge) const;

    void operator () (const Edge& edge) const;

    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const;
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

    void operator() (const boost::shared_ptr<OrientedEdge>& oe) const;
};


// Display all edges of a face
// ======================================================================
class DisplayFaceLineStrip : public DisplayElementFocus
{
public:
    DisplayFaceLineStrip (const GLWidget& widget, FocusContext focus = FOCUS,
		      bool useZPos = false, double zPos = 0) :
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }
    void operator() (const boost::shared_ptr<OrientedFace>& of);

    void operator() (const boost::shared_ptr<Face>& f);
};


/**
 * Functor that displays an edge
 */
class DisplayFaceTriangleFan : public DisplayElementFocus
{
public:
    DisplayFaceTriangleFan (const GLWidget& widget, FocusContext focus = FOCUS,
			  bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (widget, focus, useZPos, zPos)
    {
    }

    void operator () (const boost::shared_ptr<Face>& f) const;
    void operator () (const boost::shared_ptr<const OrientedFace>& of) const
    {
	operator () (of.get ());
    }
    void operator () (const OrientedFace* of) const;
};


template<typename displayEdge>
class DisplayEdges : public DisplayElementFocus
{
public:
    DisplayEdges (const GLWidget& widget, FocusContext focus, 
		  bool useZPos = false, double zPos = 0);

    void operator() (const boost::shared_ptr<OrientedFace> f);

    void operator () (const boost::shared_ptr<Face> f);
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
