/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display an edge
 */

#include "Debug.h"
#include "DebugStream.h"
#include "Disk.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "GLWidget.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "OpenGLUtils.h"
#include "Utils.h"
#include "Vertex.h"

G3D::Matrix3 edgeRotation (const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Vector3 newZ = end - begin;
    if (IsFuzzyZero (newZ))
	return G3D::Matrix3::identity ();
    newZ = newZ.unit ();
    G3D::Vector3 newX, newY;
    newZ.getTangents (newX, newY);
    G3D::Matrix3 rotation;
    rotation.setColumn (0, newX);
    rotation.setColumn (1, newY);
    rotation.setColumn (2, newZ);
    return rotation;
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

// DisplayEdge
// ======================================================================

void DisplayEdge::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end, bool context)
{
    glLineWidth (context ? m_contextRadius : m_radius);
    glBegin(GL_LINES);
    ::glVertex(begin);
    ::glVertex(end);
    glEnd();
}

// DisplayEdgeQuadric
// ======================================================================

void DisplayEdgeQuadric::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame frame (rotation, begin);
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
    glPushMatrix ();
    {
	glMultMatrix (frame);
	gluCylinder (
	    m_quadric, m_radius, m_radius, (end - begin).length (),
	    GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
    }
    glPopMatrix ();
}


// DisplayEdgeTube
// ======================================================================


Disk DisplayEdgeTube::perpendicularDisk (
    const G3D::Vector3& beginEdge, const G3D::Vector3& endEdge, 
    const G3D::Vector3& origin) const
{
    G3D::Vector3 twelveOclock, threeOclock;
    perpendicularEnd (beginEdge, endEdge, &twelveOclock, &threeOclock);
    return Disk (origin, twelveOclock, threeOclock, m_radius);
}

Disk DisplayEdgeTube::angledDisk (
    const G3D::Vector3& beforeP, const G3D::Vector3& p, 
    const G3D::Vector3& afterP,
    const G3D::Vector3& origin) const
{
    G3D::Vector3 twelveOclock, threeOclock;
    angledEnd (beforeP, p, afterP, &twelveOclock, &threeOclock);
    return Disk (origin, twelveOclock, threeOclock, m_radius);
}


void DisplayEdgeTube::operator() (const Segment& segment)
{
    Disk beginDisk, endDisk;
    switch (segment.m_perpendicularEnd)
    {
    case SegmentPerpendicularEnd::BEGIN:
    {
	beginDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				       segment.m_begin);
	endDisk = angledDisk (segment.m_begin, segment.m_end, segment.m_afterEnd,
			      segment.m_end);
	break;
    }
    case SegmentPerpendicularEnd::END:
    {
	beginDisk = angledDisk (segment.m_beforeBegin, segment.m_begin, 
				segment.m_end, segment.m_begin);
	endDisk = perpendicularDisk (segment.m_begin, segment.m_end,
				     segment.m_end);
	break;
    }
    case SegmentPerpendicularEnd::BEGIN_END:
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
    DisplayOrientedEdge () (
	disk.GetCenter (),
	disk.GetCenter () + disk.GetVertexNormal (i) * disk.GetRadius () *
	(1 + 2*static_cast<double>((i + 1)) / disk.size ()));    
}


void DisplayEdgeTube::displayTube (const Disk& begin, const Disk& end) const
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


// DisplayArrow
// ======================================================================

void DisplayArrow::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glPushAttrib (GL_LINE_BIT);
    glLineWidth (3.0);
    glBegin(GL_LINES);
    ::glVertex(begin);
    ::glVertex((begin + end) / 2);
    glEnd();
    glPopAttrib ();
}

// DisplayArrowQuadric
// ======================================================================

void DisplayArrowQuadric::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Vector3 translation;
    translation = ((m_position == BASE_MIDDLE) ?
		   (begin + end) / 2 :
		   (end - (end - begin).direction () * m_height));
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame objectToWorld (rotation, translation);
    glPushMatrix ();
    {
	glMultMatrix (objectToWorld);
	gluCylinder (m_quadric, m_baseRadius, m_topRadius, m_height,
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (m_quadric, GLU_INSIDE);
	gluDisk (m_quadric, 0, m_baseRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
    }
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
    glPopMatrix ();
}

// DisplayEdgeTorusClipped
// ======================================================================

void DisplayEdgeTorusClipped::operator() (
    const boost::shared_ptr<OrientedEdge>& oe) const
{
    operator () (oe->GetEdge ());
}


void DisplayEdgeTorusClipped::operator () (
    const boost::shared_ptr<Edge>& edge) const
{
    const OOBox& periods = m_glWidget.GetCurrentFoam ().GetOriginalDomain ();
    if (edge->IsClipped ())
    {
	glColor (edge->GetColor (Color::BLACK));
	glBegin(GL_LINES);
	for (size_t i = 0; i < edge->GetTorusClippedSize (periods); i++)
	{
	    ::glVertex(edge->GetTorusClippedBegin (i));
	    ::glVertex (edge->GetTorusClippedEnd (i));
	}
	glEnd ();
    }
}

// DisplayOrientedEdge
// ======================================================================

void DisplayOrientedEdge::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplayEdge displayEdge (m_quadric, m_topRadius);
    DisplayArrow displayArrow (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    detectOpenGLError ("dlp0");
    displayEdge (begin, end);
    detectOpenGLError ("dlp1");
    displayArrow (begin, end);
    detectOpenGLError ("dlp2");
}

// DisplayOrientedEdgeQuadric
// ======================================================================

void DisplayOrientedEdgeQuadric::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplayEdgeQuadric displayEdge (m_quadric, m_topRadius);
    DisplayArrowQuadric displayArrow (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, end);
    displayArrow (begin, end);
}

// DisplayEdgeTorus
// ======================================================================
template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
DisplayEdgeTorus<DisplayEdge, DisplayArrow, showDuplicates>::DisplayEdgeTorus (
    const GLWidget& widget, FocusContext focus, bool useZPos, double zPos) : 
    
    DisplayElementFocus (widget, focus, useZPos, zPos),
    m_displayEdge (m_glWidget.GetQuadricObject (), m_glWidget.GetEdgeRadius ()),
    m_displayArrow (m_glWidget.GetQuadricObject (),
		    m_glWidget.GetArrowBaseRadius (),
		    m_glWidget.GetEdgeRadius (),
		    m_glWidget.GetArrowHeight ())
    {
    }

template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplayArrow, showDuplicates>::
operator () (const OrientedEdge& oe)
{
    operator () (oe.GetEdge ());
}

template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplayArrow, showDuplicates>::
operator() (const boost::shared_ptr<OrientedEdge> oe) 
{
    operator() (oe->GetEdge());
}

template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplayArrow, showDuplicates>::
operator() (const boost::shared_ptr<Edge>  e)
{
    if (showDuplicates 
	|| e->GetDuplicateStatus () != ElementStatus::DUPLICATE)
	display (e);
}

template <typename DisplayEdge, typename DisplayArrow, bool showDuplicates>
void DisplayEdgeTorus<DisplayEdge, DisplayArrow, showDuplicates>::
display (const boost::shared_ptr<Edge>  e)
{
    glPushAttrib (GL_CURRENT_BIT);
    const Vertex* begin = e->GetBegin ().get ();
    const Vertex* end = e->GetEnd ().get ();
    G3D::Vector3int16 endLocation = e->GetEndTranslation ();
    glColor (m_glWidget.GetEndTranslationColor (endLocation));

    if (endLocation != Vector3int16Zero)
	m_displayArrow(begin->GetVector (), end->GetVector ());
    m_displayEdge(begin->GetVector (), end->GetVector ());
    glPopAttrib ();
}


// DisplayEdgeWithColor
// ======================================================================

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
DisplayEdgeWithColor<tesselationEdgesDisplay>::
DisplayEdgeWithColor (const GLWidget& widget, FocusContext focus,
		      bool useZPos, double zPos) : 
    
    DisplayElementFocus (widget, focus, useZPos, zPos)
{
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgeWithColor<tesselationEdgesDisplay>::
operator () (const boost::shared_ptr<Edge> edge) const
{
    operator () (*edge);
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgeWithColor<tesselationEdgesDisplay>::
operator () (const Edge& edge) const
{
    const Foam& foam = m_glWidget.GetCurrentFoam ();
    bool isPhysical = edge.IsPhysical (foam.Is2D (), foam.IsQuadratic ());
    if (isPhysical || 
	(tesselationEdgesDisplay == TEST_DISPLAY_TESSELLATION &&
	 m_glWidget.IsEdgesTessellation () && 
	 m_focus == FOCUS))
    {
	double alpha = 
	    (m_focus == FOCUS ? 1.0 : m_glWidget.GetContextAlpha ());
	QColor color = edge.GetColor (Qt::black);
	glColor (
	    QColor::fromRgbF(
		color.redF (), color.greenF (), color.blueF (), alpha));
	glBegin(GL_LINE_STRIP);
	DisplayEdgeVertices (edge, m_useZPos, m_zPos);
	glEnd ();
    }
}

template <DisplayElement::TessellationEdgesDisplay tesselationEdgesDisplay>
void DisplayEdgeWithColor<tesselationEdgesDisplay>::
operator() (const boost::shared_ptr<OrientedEdge> oe) const
    {
	operator () (oe->GetEdge ());
    }


// DisplayFaceLineStrip
// ======================================================================
void DisplayFaceLineStrip::operator() (const boost::shared_ptr<OrientedFace>&  of)
{
    operator() (of->GetFace ());
}

void DisplayFaceLineStrip::operator() (const boost::shared_ptr<Face>& f)
{
    glBegin (GL_LINE_STRIP);
    const vector<boost::shared_ptr<OrientedEdge> >& v =
	f->GetOrientedEdges ();
    for_each (v.begin (), v.end (), DisplayAllVertices);
    glEnd ();
}


// DisplayFaceTriangleFan
// ======================================================================

void DisplayFaceTriangleFan::operator() (
    const boost::shared_ptr<Face>& f) const
{
    OrientedFace of (f, false);
    operator () (&of);
}

void DisplayFaceTriangleFan::operator() (const OrientedFace*  of) const
{
    OrientedEdge oe = of->GetOrientedEdge (0);
    glBegin (GL_TRIANGLE_FAN);
    ::glVertex (of->GetCenter ());
    ::glVertex (oe.GetPoint (0));
    ::glVertex (oe.GetPoint (1));
    size_t pointIndex = 2;
    for (size_t i = 0; i < of->size (); ++i)
    {
	oe = of->GetOrientedEdge (i);
	for (; pointIndex < oe.GetPointCount (); ++pointIndex)
	    ::glVertex (oe.GetPoint (pointIndex));
	pointIndex = 0;
    }
	
    if (! of->IsClosed ())
	::glVertex (of->GetOrientedEdge (0).GetBegin ()->GetVector ());
    glEnd ();
}


// DisplayEdges
// ======================================================================

template<typename displayEdge>
DisplayEdges<displayEdge>::
DisplayEdges (const GLWidget& widget, FocusContext focus, bool useZPos, 
	      double zPos) :
 
    DisplayElementFocus (widget, focus, useZPos, zPos)
{
}

template<typename displayEdge>
void DisplayEdges<displayEdge>::
operator() (const boost::shared_ptr<OrientedFace>  f)
{
    operator() (f->GetFace ());
}

template<typename displayEdge>
void DisplayEdges<displayEdge>::
operator () (const boost::shared_ptr<Face>  f)
{
    const vector< boost::shared_ptr<OrientedEdge> >& v = 
	f->GetOrientedEdges ();
    displayEdge display(m_glWidget, m_focus, m_useZPos, m_zPos);
    for (size_t i = 0; i < v.size (); i++)
    {
	boost::shared_ptr<OrientedEdge> oe = v[i];
	size_t displayedEdgeIndex = m_glWidget.GetSelectedEdgeIndex ();
	if (m_glWidget.IsDisplayedEdge (i))
	{
	    display (oe);
	    if (i == displayedEdgeIndex)
		cdbg << "edge " << i << ": " << *oe;
	}
    }
}

// Template instantiations
// ======================================================================

// DisplayEdgeTorus
// ======================================================================

template class DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, false>;
template class DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true>;
template class DisplayEdgeTorus<DisplayEdge, DisplayArrow, false>;
template class DisplayEdgeTorus<DisplayEdge, DisplayArrow, true>;


// DisplayEdgeWithColor
// ======================================================================

template class DisplayEdgeWithColor <DisplayElement::TEST_DISPLAY_TESSELLATION>;
template class DisplayEdgeWithColor <DisplayElement::DONT_DISPLAY_TESSELLATION>;


// DisplayEdgeTorus
// ======================================================================

template class DisplayEdges<
    DisplayEdgeTorus <DisplayEdge, DisplayArrow, true> >;
template class DisplayEdges<
    DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true> >;
template class DisplayEdges<
    DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >;
template class DisplayEdges<
    DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >;
template class DisplayEdges<DisplayEdgeTorusClipped>;

