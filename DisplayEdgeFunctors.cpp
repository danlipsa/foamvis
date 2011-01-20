/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display an edge
 */

#include "DebugStream.h"
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
    if (isFuzzyZero (newZ))
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

// DisplayEdge
// ======================================================================

void DisplayEdge::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex(end);
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
	    m_quadric, m_edgeRadius, m_edgeRadius, (end - begin).length (),
	    GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
    }
    glPopMatrix ();
}




// DisplayArrow
// ======================================================================

void DisplayArrow::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glPushAttrib (GL_LINE_BIT);
    glLineWidth (2.0);
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex((begin + end) / 2);
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

// DisplayOrientedEdge
// ======================================================================

void DisplayOrientedEdge::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplayEdge displayEdge (m_quadric, m_topRadius);
    DisplayArrow displayArrow (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, end);
    displayArrow (begin, end);
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
	m_displayArrow(*begin, *end);
    m_displayEdge(*begin, *end);
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


// DisplaySameEdges
// ======================================================================
void DisplaySameEdges::operator() (const boost::shared_ptr<OrientedFace>&  of)
{
    operator() (of->GetFace ());
}

void DisplaySameEdges::operator() (const boost::shared_ptr<Face>& f)
{
    glBegin (m_primitive);
    const vector<boost::shared_ptr<OrientedEdge> >& v =
	f->GetOrientedEdges ();
    for_each (v.begin (), v.end (), DisplayAllButLastVertices);
    if (! f->IsClosed ())
	glVertex (*v[v.size () - 1]->GetEnd ());
    glEnd ();
}

// DisplayTriangleFan
// ======================================================================

void DisplayTriangleFan::operator() (const boost::shared_ptr<OrientedFace>&  of)
{
    operator() (of->GetFace ());
}


// DisplaySameTriangle
// ======================================================================

void DisplaySameTriangles::operator() (const boost::shared_ptr<Face>& f)
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

void DisplaySameTriangles::operator() (const boost::shared_ptr<OrientedFace>& of)
{
    operator() (of->GetFace ());
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
	size_t displayedEdgeIndex = m_glWidget.GetDisplayedEdgeIndex ();
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

