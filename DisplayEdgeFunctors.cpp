/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display an edge
 */

#include "Debug.h"
#include "Disk.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "Settings.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "OpenGLUtils.h"
#include "Utils.h"
#include "Vertex.h"
#include "ViewSettings.h"

void DisplayEdgeVertices (const Edge& edge, bool useZPos, double zPos)
{
    for (size_t i = 0; i < edge.GetPointCount (); ++i)
    {
	G3D::Vector3 p = edge.GetPoint (i);
	if (useZPos)
	    p = G3D::Vector3 (p.xy (), zPos);
	::glVertex (p);
    }
}


void DisplayEdgeVerticesNoEnds (const Edge& edge)
{
    for (size_t i = 1; i < edge.GetPointCount () - 1; ++i)
	::glVertex (edge.GetPoint (i));
}

void perpendicularEnd (const G3D::Vector3& normal,
		       G3D::Vector3* twelveOclock, G3D::Vector3* threeOclock)
{
    G3D::Plane plane (normal, G3D::Vector3::zero ());
    *twelveOclock = plane.closestPoint (G3D::Vector3::unitX ()).unit ();
    *threeOclock = twelveOclock->cross (normal);
}


void perpendicularEnd (const G3D::Vector3& begin, const G3D::Vector3& end,
		       G3D::Vector3* twelveOclock, G3D::Vector3* threeOclock)
{
    G3D::Vector3 normal = (end - begin).unit ();
    perpendicularEnd (normal, twelveOclock, threeOclock);
}

void angledEnd (const G3D::Vector3& before, 
		const G3D::Vector3& p,
		const G3D::Vector3& after,
		G3D::Vector3* twelveOclock, G3D::Vector3* threeOclock)
{
    G3D::Vector3 firstNormal = (p - before).unit ();
    G3D::Vector3 secondNormal = (after - p).unit ();
    G3D::Vector3 normal = (firstNormal + secondNormal).unit ();
    perpendicularEnd (normal, twelveOclock, threeOclock);
}

// DisplaySegmentArrow2D
// ======================================================================
void DisplaySegmentArrow2D (G3D::Vector2 where, G3D::Vector2 v, 
			  float lineWidth, 
			  float onePixelInObjectSpace, bool clamped)
{
    const float arrowDegrees = 15.0;
    const float arrowLengthInPixels = 10;
    G3D::Vector2 center = where + v / 2;
    float arrowLength = min (
	v.length (), arrowLengthInPixels * onePixelInObjectSpace);
    G3D::Vector2 arrow = v.direction () * arrowLength;
    glPushMatrix ();
    glLineWidth (lineWidth);
    glTranslate (center);
    glBegin (GL_LINES);
    ::glVertex (- v / 2);
    ::glVertex (v / 2);
    glEnd ();
    glTranslate (v / 2);
    glBegin (GL_TRIANGLES);
    ::glVertex (- rotateDegrees (arrow, arrowDegrees));
    ::glVertex (G3D::Vector2::zero ());
    ::glVertex (- rotateDegrees (arrow, -arrowDegrees));
    glEnd ();
    if (clamped)
    {
	glTranslate (- v / 2);
	arrow = G3D::Vector2 (- arrow.y, arrow.x) /2;
	glBegin (GL_LINES);
	::glVertex (- arrow);
	::glVertex (arrow);
	glEnd ();
    }
    glPopMatrix ();
    glLineWidth (1.0);
}

// DisplayVtkArrow
// ======================================================================
void DisplayVtkArrow (GLUquadricObj* quadric)
{
    DisplayArrowQuadric (quadric, 0.1, 0.03, 0.35)
        (G3D::Vector3::zero (), G3D::Vector3 (1.0, 0, 0));
}


// DisplaySegmentLine
// ======================================================================
void DisplaySegmentLine::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end, bool context)
{
    glLineWidth (context ? m_contextRadius : m_radius);
    glBegin(GL_LINES);
    ::glVertex(begin);
    ::glVertex(end);
    glEnd();
    glLineWidth (1.0);
}

// DisplaySegmentQuadric
// ======================================================================

void DisplaySegmentQuadric::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Matrix3 rotation = GetAxisRotation (begin, end, G3D::Vector3::Z_AXIS);
    G3D::CoordinateFrame frame (rotation, begin);
    glPushMatrix ();
    {
	glMultMatrix (frame);
	gluCylinder (
	    m_quadric, m_radius, m_radius, (end - begin).length (),
	    Settings::QUADRIC_SLICES, Settings::QUADRIC_STACKS);
    }
    glPopMatrix ();
}


// DisplaySegmentTube
// ======================================================================
Disk DisplaySegmentTube::perpendicularDisk (
    const G3D::Vector3& beginEdge, const G3D::Vector3& endEdge, 
    const G3D::Vector3& origin) const
{
    G3D::Vector3 twelveOclock, threeOclock;
    perpendicularEnd (beginEdge, endEdge, &twelveOclock, &threeOclock);
    return Disk (origin, twelveOclock, threeOclock, m_radius);
}

Disk DisplaySegmentTube::angledDisk (
    const G3D::Vector3& beforeP, const G3D::Vector3& p, 
    const G3D::Vector3& afterP,
    const G3D::Vector3& origin) const
{
    G3D::Vector3 twelveOclock, threeOclock;
    angledEnd (beforeP, p, afterP, &twelveOclock, &threeOclock);
    return Disk (origin, twelveOclock, threeOclock, m_radius);
}


void DisplaySegmentTube::operator() (const Segment& segment)
{
    Disk beginDisk, endDisk;
    switch (segment.m_perpendicularEnd)
    {
    case SegmentPerpendicularEnd::BEGIN_SEGMENT:
    {
	beginDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				       segment.m_begin);
	endDisk = angledDisk (segment.m_begin, segment.m_end, 
			      segment.m_afterEnd, segment.m_end);
	break;
    }
    case SegmentPerpendicularEnd::END_SEGMENT:
    {
	beginDisk = angledDisk (segment.m_beforeBegin, segment.m_begin, 
				segment.m_end, segment.m_begin);
	endDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				     segment.m_end);
	break;
    }
    case SegmentPerpendicularEnd::BEGIN_END_SEGMENT:
    {
	beginDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				       segment.m_begin);
	endDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				     segment.m_end);
	break;
    }
    case SegmentPerpendicularEnd::NONE:
    {
	beginDisk = angledDisk (
	    segment.m_beforeBegin, segment.m_begin, segment.m_end, 
	    segment.m_begin);
	endDisk = angledDisk (
	    segment.m_begin, segment.m_end, segment.m_afterEnd, segment.m_end);
	break;
    }

    default:
	ThrowException ("Invalid SegmentPerpendicularEnd::Enum: ", 
			segment.m_perpendicularEnd);
    }
    displayTube (beginDisk, endDisk);
}

void displayNormal (const Disk& disk, size_t i)
{
    DisplayOrientedSegmentLine () (
	disk.GetCenter (),
	disk.GetCenter () + disk.GetVertexNormal (i) * disk.GetRadius () *
	(1 + 2*static_cast<double>((i + 1)) / disk.size ()));    
}


void DisplaySegmentTube::displayTube (const Disk& begin, const Disk& end) const
{
    glBegin (GL_QUAD_STRIP);
    for (size_t i = 0; i < begin.size (); ++i)
    {
	glNormal (begin.GetVertexNormal (i));
	::glVertex (begin.GetVertex (i));
	glNormal (end.GetVertexNormal (i));
	::glVertex (end.GetVertex (i));
    }
    glNormal (begin.GetVertexNormal (0));
    ::glVertex (begin.GetVertex (0));
    glNormal (end.GetVertexNormal (0));
    ::glVertex (end.GetVertex (0));
    glEnd ();    
}


// DisplaySegmentArrow1
// ======================================================================

DisplaySegmentArrow1::DisplaySegmentArrow1 (
    GLUquadricObj* quadric,
    double baseRadius, double topRadius, double height,
    ArrowHeadPosition position) :

    m_quadric (quadric),
    m_baseRadius (baseRadius),
    m_topRadius(topRadius),
    m_height (height),
    m_position (position)
{
}

void DisplaySegmentArrow1::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glLineWidth (3.0);
    glBegin(GL_LINES);
    ::glVertex(begin);
    ::glVertex((begin + end) / 2);
    glEnd();
    glLineWidth (1.0);
}

// DisplayArrowHeadQuadric
// ======================================================================

DisplayArrowHeadQuadric::DisplayArrowHeadQuadric (
    GLUquadricObj* quadric, 
    double baseRadius, double topRadius, double height, 
    ArrowHeadPosition position) :

    DisplaySegmentArrow1 (quadric, baseRadius, topRadius, height, position)
{
}

G3D::Vector3 DisplayArrowHeadQuadric::GetBasePosition (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    return ((m_position == BASE_MIDDLE) ?
            (begin + end) / 2 :
            (end - (end - begin).direction () * m_height));
}


void DisplayArrowHeadQuadric::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Vector3 translation = GetBasePosition (begin, end);
    G3D::Matrix3 rotation = GetAxisRotation (begin, end, G3D::Vector3::Z_AXIS);
    G3D::CoordinateFrame objectToWorld (rotation, translation);
    glPushMatrix ();
    {
	glMultMatrix (objectToWorld);
	gluCylinder (m_quadric, m_baseRadius, 
                     m_position == BASE_MIDDLE ? m_topRadius : 0, m_height,
		     Settings::QUADRIC_SLICES, Settings::QUADRIC_STACKS);
	gluQuadricOrientation (m_quadric, GLU_INSIDE);
	gluDisk (m_quadric, 0, m_baseRadius, Settings::QUADRIC_SLICES,
		 Settings::QUADRIC_STACKS);
        gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
    }
    glPopMatrix ();
}

// DisplayArrowQuadric
// ======================================================================

DisplayArrowQuadric::DisplayArrowQuadric (
    GLUquadricObj* quadric,
    double baseRadius, double topRadius, double height,
    ArrowHeadPosition position) :

    DisplaySegmentArrow1 (quadric, baseRadius, topRadius, height, position)
{
}

void DisplayArrowQuadric::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplaySegmentQuadric displayEdge (m_quadric, m_topRadius);
    DisplayArrowHeadQuadric displayArrowHead (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, m_position == BASE_MIDDLE ? end : 
                 displayArrowHead.GetBasePosition (begin, end));
    displayArrowHead (begin, end);
}


// DisplayOrientedSegmentLine
// ======================================================================

void DisplayOrientedSegmentLine::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplaySegmentLine displayEdge (m_quadric, m_topRadius);
    DisplaySegmentArrow1 displayThickHalf (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, end);
    displayThickHalf (begin, end);
}

// DisplayEdgeTorus
// ======================================================================
template <typename DisplayEdge, typename DisplaySegmentArrow1, 
	  bool showDuplicates>
DisplayEdgeTorus<DisplayEdge, DisplaySegmentArrow1, showDuplicates>::
DisplayEdgeTorus (
    const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
    FocusContext focus, bool useZPos, double zPos, 
    GLUquadricObj* quadric) : 
    
    DisplayElementFocus (settings, viewNumber, is2D, focus, useZPos, zPos),
    m_displayEdge (quadric, m_settings.GetEdgeRadius ()),
    m_displayArrow (quadric,
		    m_settings.GetArrowBaseRadius (),
		    m_settings.GetEdgeRadius (),
		    m_settings.GetArrowHeight ())
    {
    }

template <typename DisplayEdge, typename DisplaySegmentArrow1, 
	  bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplaySegmentArrow1, showDuplicates>::
operator () (const OrientedEdge& oe)
{
    operator () (oe.GetEdge ());
}

template <typename DisplayEdge, typename DisplaySegmentArrow1, 
	  bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplaySegmentArrow1, showDuplicates>::
operator() (const boost::shared_ptr<OrientedEdge> oe) 
{
    operator() (oe->GetEdge());
}

template <typename DisplayEdge, typename DisplaySegmentArrow1, 
	  bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplaySegmentArrow1, showDuplicates>::
operator() (const boost::shared_ptr<Edge>  e)
{
    if (showDuplicates 
	|| e->GetDuplicateStatus () != ElementStatus::DUPLICATE)
	display (e);
}

template <typename DisplayEdge, typename DisplaySegmentArrow1, 
	  bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplaySegmentArrow1, showDuplicates>::
display (const boost::shared_ptr<Edge>  e)
{
    glPushAttrib (GL_CURRENT_BIT);
    const Vertex& begin = e->GetBegin ();
    const Vertex& end = e->GetEnd ();
    G3D::Vector3int16 endLocation = e->GetEndTranslation ();
    glColor (m_settings.GetEndTranslationColor (endLocation));

    if (endLocation != Vector3int16Zero)
	m_displayArrow(begin.GetVector (), end.GetVector ());
    m_displayEdge(begin.GetVector (), end.GetVector ());
    glPopAttrib ();
}


// DisplayEdgePropertyColor
// ======================================================================

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
DisplayEdgePropertyColor<tesselationEdgesDisplay>::
DisplayEdgePropertyColor (
    const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
    FocusContext focus, bool useZPos, double zPos) : 
    
    DisplayElementFocus (settings, viewNumber, is2D, focus, useZPos, zPos)
{
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgePropertyColor<tesselationEdgesDisplay>::
operator () (const boost::shared_ptr<Edge> edge) const
{
    operator () (*edge);
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgePropertyColor<tesselationEdgesDisplay>::
operator () (const Edge& edge) const
{
    ViewSettings& vs = this->m_settings.GetViewSettings (m_viewNumber);
    bool isPhysical = edge.IsPhysical (m_is2D);
    if (isPhysical || 
	(tesselationEdgesDisplay == DISPLAY_TESSELLATION_EDGES &&
	 m_settings.EdgesTessellationShown () && 
	 m_focus == FOCUS))
    {
	bool hasConstraints = edge.HasConstraints ();
	if (hasConstraints && ! m_settings.ConstraintsShown ())
	    return;
	double alpha = 
	    (m_focus == FOCUS ? 1.0 : vs.GetContextAlpha ());
	QColor color = edge.GetColor (Qt::black);
	glColor (
	    QColor::fromRgbF(
		color.redF (), color.greenF (), color.blueF (), alpha));
	glBegin(GL_LINE_STRIP);
	DisplayEdgeVertices (edge, m_useZPos, m_zPos);
	glEnd ();
	
	if (hasConstraints && m_settings.ConstraintPointsShown ())
	{
	    glPointSize (5);
	    glBegin(GL_POINTS);
	    DisplayEdgeVerticesNoEnds (edge);
	    glEnd ();
	}
    }
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgePropertyColor<tesselationEdgesDisplay>::
operator() (const boost::shared_ptr<OrientedEdge> oe) const
{
    operator () (oe->GetEdge ());
}


// DisplayEdge
// ======================================================================

DisplayEdge::DisplayEdge (
    const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
    FocusContext focus, 
    bool useZPos, double zPos) : 
    
    DisplayElementFocus (settings, viewNumber, is2D, focus, useZPos, zPos)
{
}

void DisplayEdge::operator () (const boost::shared_ptr<Edge> edge) const
{
    operator () (*edge);
}

void DisplayEdge::operator () (const Edge& edge) const
{
    glBegin(GL_LINE_STRIP);
    DisplayEdgeVertices (edge, m_useZPos, m_zPos);
    glEnd ();
}

void DisplayEdge::operator() (const boost::shared_ptr<OrientedEdge> oe) const
{
    operator () (oe->GetEdge ());
}


// Template instantiations
// ======================================================================

// DisplayEdgeTorus
// ======================================================================

template class DisplayEdgeTorus<DisplaySegmentQuadric, DisplayArrowHeadQuadric, false>;
template class DisplayEdgeTorus<DisplaySegmentQuadric, DisplayArrowHeadQuadric, true>;
template class DisplayEdgeTorus<DisplaySegmentLine, DisplaySegmentArrow1, false>;
template class DisplayEdgeTorus<DisplaySegmentLine, DisplaySegmentArrow1, true>;


// DisplayEdgePropertyColor
// ======================================================================

template class DisplayEdgePropertyColor <DisplayElement::DISPLAY_TESSELLATION_EDGES>;
template class DisplayEdgePropertyColor <DisplayElement::DONT_DISPLAY_TESSELLATION_EDGES>;



