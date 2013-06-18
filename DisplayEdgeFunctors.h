/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 * @brief Functors that display an edge
 */

#ifndef __DISPLAY_EDGE_FUNCTORS_H__
#define __DISPLAY_EDGE_FUNCTORS_H__

#include "DisplayElement.h"
class Disk;
class Edge;
class Face;
class OrientedEdge;
class OrientedFace;

void DisplayEdgeVertices (const Edge& edge,
			  bool useZPos = false, double zPos = 0);
void DisplayEdgeVerticesNoEnds (const Edge& edge);
void DisplayOrientedEdgeVertices (const boost::shared_ptr<OrientedEdge> oe);

void DisplaySegmentArrow2D (G3D::Vector2 v, G3D::Vector2 center,
                            float lineWidth, 
                            float onePixelInObjectSpace, bool clamped);
/**
 * @brief Display an arrow in VTK style.
 *
 * Display an arrow from 0,1 along X axis, with shaft = 0.03, 
 * arrowHeadRadius = 0.1, arrowHeadHeight = 0.35.
 * if quadric == 0 it draws a 2D arrow.
 */
void DisplayVtkArrow (G3D::Vector3 where, G3D::Vector3 v, 
                      GLUquadricObj* quadric = 0);
/**
 * @brief Represents a segment used to display bubble paths
 */
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

/**
 * @brief Displays a segment as a line
 */
class DisplaySegmentLine
{
public:
    DisplaySegmentLine () :
	m_quadric (0), m_radius (1.0), m_contextRadius (1.0)
    {
    }
    

    DisplaySegmentLine (GLUquadricObj* quadric, double edgeRadius, 
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

/**
 * @brief Displays a segment as a tube using a quadric
 */
class DisplaySegmentQuadric : public DisplaySegmentLine
{
public:
    DisplaySegmentQuadric () : DisplaySegmentLine ()
    {
    }

    DisplaySegmentQuadric (
	GLUquadricObj* quadric, double edgeRadius, double contextRadius = 2.0) :
	DisplaySegmentLine (quadric, edgeRadius, contextRadius)
    {
    }
    
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
    void operator () (const Segment& segment)
    {
	operator () (segment.m_begin, segment.m_end);
    }
};


/**
 * @brief Displays a segment as a tube using OpenGL
 */
class DisplaySegmentTube : public DisplaySegmentLine
{
public:
    DisplaySegmentTube () : DisplaySegmentLine ()
    {
    }

    DisplaySegmentTube (GLUquadricObj* quadric, double edgeRadius, 
		     double contextRadius = 2.0) :
	DisplaySegmentLine (quadric, edgeRadius, contextRadius)
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


/**
 * @brief Displays the first half of a segment using a thick line.
 */
class DisplayThickFirstHalf
{
public:
    enum ArrowHeadPosition
    {
	BASE_MIDDLE,
	TOP_END
    };

public:
    DisplayThickFirstHalf (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 0, double height = 0,
	ArrowHeadPosition position = BASE_MIDDLE);
    void operator () (const G3D::Vector3& begin, const G3D::Vector3& end);

protected:
    

protected:
    GLUquadricObj* m_quadric;
    double m_arrowHeadRadius;
    double m_arrowHeadHeight;
    double m_shaftRadius;
    ArrowHeadPosition m_position;
};


/**
 * @brief Displays an arrow head using quadrics
 */
class DisplayArrowHeadQuadric : public DisplayThickFirstHalf
{
public:
    DisplayArrowHeadQuadric (
	GLUquadricObj* quadric,
	double arrowHeadRadius, double shaftRadius, double arrowHeadHeight, 
	ArrowHeadPosition position = TOP_END);
    void operator () (const G3D::Vector3& begin, const G3D::Vector3& end);
    G3D::Vector3 GetBasePosition (
        const G3D::Vector3& begin, const G3D::Vector3& end);
};

/**
 * @brief Displays an oriented segment with a thick first half
 */
class DisplayOrientedSegmentLine : public DisplayThickFirstHalf
{
public:
    DisplayOrientedSegmentLine (
	GLUquadricObj* quadric = 0,
	double baseRadius = 0, double topRadius = 1.0, double height = 0,
	ArrowHeadPosition position = BASE_MIDDLE) :
	DisplayThickFirstHalf (quadric, baseRadius, topRadius, height, position)
    {
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};


/**
 * @brief Displays an arrow using a quadric
 */
class DisplayArrowQuadric : public DisplayThickFirstHalf
{
public:
    DisplayArrowQuadric (
	GLUquadricObj* quadric = 0,
	double arrowHeadRadius = 0, double shaftRadius = 0, 
        double arrowHeadHeight = 0, ArrowHeadPosition position = TOP_END);
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end);
};


/**
 * @brief Displays an oriented edge that intersects the torus domain
 *        encoding the boundaries intersected using color.
 */
template <typename DisplayEdge, 
	  typename DisplayThickFirstHalf, bool showDuplicates>
class DisplayEdgeTorus : public DisplayElementFocus
{
public:
    DisplayEdgeTorus (
        const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
        FocusContext focus = FOCUS,
        bool useZPos = false, double zPos = 0, 
        GLUquadricObj* quadric = 0);

    void operator () (const OrientedEdge& oe);

    void operator() (const boost::shared_ptr<OrientedEdge> oe);
    
    void operator() (const boost::shared_ptr<Edge>  e);

protected:
    void display (const boost::shared_ptr<Edge>  e);

private:
    DisplayEdge m_displayEdge;
    DisplayThickFirstHalf m_displayArrow;
};

/**
 * @brief Displays an edge colored by its color property
 */
template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay = 
	  DisplayElement::DISPLAY_TESSELLATION_EDGES>
class DisplayEdgePropertyColor : public DisplayElementFocus
{
public:
    DisplayEdgePropertyColor (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	FocusContext focus, bool useZPos = false, double zPos = 0);

    void operator () (const boost::shared_ptr<Edge> edge) const;

    void operator () (const Edge& edge) const;

    void operator() (const boost::shared_ptr<OrientedEdge> oe) const;
};


/**
 * @brief Displays an edge as a line strip
 */
class DisplayEdge : public DisplayElementFocus
{
public:
    DisplayEdge (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
        FocusContext focus, 
	bool useZPos = false, double zPos = 0);

    void operator () (const boost::shared_ptr<Edge> edge) const;
    void operator () (const Edge& edge) const;
    void operator() (const boost::shared_ptr<OrientedEdge> oe) const;
private:
    ViewNumber::Enum m_viewNumber;
};

#endif //__DISPLAY_EDGE_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
