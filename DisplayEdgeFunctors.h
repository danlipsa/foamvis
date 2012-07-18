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

// Display segments
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


class DisplaySegment
{
public:
    DisplaySegment () :
	m_quadric (0), m_radius (1.0), m_contextRadius (1.0)
    {
    }
    

    DisplaySegment (GLUquadricObj* quadric, double edgeRadius, 
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

class DisplaySegmentQuadric : public DisplaySegment
{
public:
    DisplaySegmentQuadric () : DisplaySegment ()
    {
    }

    DisplaySegmentQuadric (
	GLUquadricObj* quadric, double edgeRadius, double contextRadius = 2.0) :
	DisplaySegment (quadric, edgeRadius, contextRadius)
    {
    }
    
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
    void operator () (const Segment& segment)
    {
	operator () (segment.m_begin, segment.m_end);
    }
};

class DisplaySegmentTube : public DisplaySegment
{
public:
    DisplaySegmentTube () : DisplaySegment ()
    {
    }

    DisplaySegmentTube (GLUquadricObj* quadric, double edgeRadius, 
		     double contextRadius = 2.0) :
	DisplaySegment (quadric, edgeRadius, contextRadius)
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

void DisplaySegmentArrow (G3D::Vector2 v, G3D::Vector2 center,
			  float lineWidth, 
			  float onePixelInObjectSpace, bool clamped);

class DisplaySegmentArrow1
{
public:
    enum Position
    {
	BASE_MIDDLE,
	TOP_END
    };

public:
    DisplaySegmentArrow1 (
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

class DisplaySegmentArrowQuadric : public DisplaySegmentArrow1
{
public:
    DisplaySegmentArrowQuadric (
	GLUquadricObj* quadric,
	double baseRadius, double topRadius, double height, 
	Position position = BASE_MIDDLE) :
	DisplaySegmentArrow1 (quadric, baseRadius, topRadius, height, position)
    {
    }

    void operator () (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayOrientedSegment : public DisplaySegmentArrow1
{
public:
    DisplayOrientedSegment (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 1.0, double height = 0,
	Position position = BASE_MIDDLE) :
	DisplaySegmentArrow1 (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

class DisplayOrientedSegmentQuadric : public DisplaySegmentArrow1
{
public:
    DisplayOrientedSegmentQuadric (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 0, double height = 0,
	Position position = BASE_MIDDLE) :
	DisplaySegmentArrow1 (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};

// Display one edge
// ======================================================================

template <typename DisplayEdge, 
	  typename DisplaySegmentArrow1, bool showDuplicates>
class DisplayEdgeTorus : public DisplayElementFocus
{
public:
    DisplayEdgeTorus (const Settings& settings, const Foam& foam, 
		      GLUquadricObj* quadric,
		      FocusContext focus = FOCUS, 
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
    DisplaySegmentArrow1 m_displayArrow;
};

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay = 
	  DisplayElement::DISPLAY_TESSELLATION_EDGES>
class DisplayEdgePropertyColor : public DisplayElementFocus
{
public:
    DisplayEdgePropertyColor (
	const Settings& settings, const Foam& foam,
	FocusContext focus, bool useZPos = false, double zPos = 0);

    DisplayEdgePropertyColor (
	const Settings& settings, const Foam& foam,
	FocusContext focus, ViewNumber::Enum viewNumber, 
	bool useZPos = false, double zPos = 0);

    void operator () (const boost::shared_ptr<Edge> edge) const;

    void operator () (const Edge& edge) const;

    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const;
};


template <HighlightNumber::Enum highlightNumber,
	  DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay = 
	  DisplayElement::DISPLAY_TESSELLATION_EDGES>
class DisplayEdgeHighlightColor : public DisplayElementFocus
{
public:
    DisplayEdgeHighlightColor (
	const Settings& settings, const Foam& foam,
	FocusContext focus, ViewNumber::Enum viewNumber,
	bool useZPos = false, double zPos = 0);

    void operator () (const boost::shared_ptr<Edge> edge) const;
    void operator () (const Edge& edge) const;
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const;
private:
    ViewNumber::Enum m_viewNumber;
};



/**
 * Used for displaying edges clipped at the torus original domain boudary.
 * @todo Use circular arcs in this view as well.
 */
class DisplayEdgeTorusClipped : public DisplayElementFocus
{
public:
    DisplayEdgeTorusClipped (const Settings& settings, const Foam& foam,
			     FocusContext focus, 
			     bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (settings, foam, focus, useZPos, zPos)
    {
    }

    DisplayEdgeTorusClipped (const Settings& settings, const Foam& foam,
			     FocusContext focus, ViewNumber::Enum viewNumber,
			     bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (settings, foam, focus, useZPos, zPos)
    {
	(void)viewNumber;
    }

    void operator () (const boost::shared_ptr<Edge>& edge) const;

    void operator() (const boost::shared_ptr<OrientedEdge>& oe) const;
};


// Display all edges of a face
// ======================================================================
class DisplayFaceLineStrip : public DisplayElementFocus
{
public:
    DisplayFaceLineStrip (
	const Settings& settings, const Foam& foam,
	FocusContext focus = FOCUS,
	bool useZPos = false, double zPos = 0) :
	DisplayElementFocus (settings, foam, focus, useZPos, zPos)
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
    DisplayFaceTriangleFan (
	const Settings& settings, const Foam& foam,
	FocusContext focus = FOCUS,
	bool useZPos = false, double zPos = 0) : 
	DisplayElementFocus (settings, foam, focus, useZPos, zPos)
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
class DisplayFaceEdges : public DisplayElementFocus
{
public:
    DisplayFaceEdges (const Settings& settings, const Foam& foam,
		      FocusContext focus, 
		      bool useZPos = false, double zPos = 0);

    void operator() (const boost::shared_ptr<OrientedFace> f);

    void operator () (const boost::shared_ptr<Face> f);
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
