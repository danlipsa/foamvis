/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Implementation for functors that display a face
 */

#include "Body.h"
#include "DataProperties.h"
#include "Debug.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Face.h"
#include "Foam.h"
#include "Settings.h"
#include "OpenGLUtils.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"
#include "ViewSettings.h"


void  DisplayOrientedEdgeVertices (const boost::shared_ptr<OrientedEdge> e)
{
    for (size_t i = 0; i < e->GetPointCount (); ++i)
	::glVertex(e->GetPoint (i));
}


// DisplayFaceHighlightColor
// ======================================================================

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceHighlightColor<highlightColorIndex, 
			  displayEdges, PropertySetter>::
DisplayFaceHighlightColor (
    const Settings& settings, bool is2D,
    typename DisplayElement::FocusContext focus,
    ViewNumber::Enum viewNumber, bool useZPos, double zPos) : 
    
    DisplayElementPropertyFocus<PropertySetter> (
	settings, 
        PropertySetter (settings, viewNumber, is2D), 
        focus, useZPos, zPos)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceHighlightColor<highlightColorIndex, 
			  displayEdges, PropertySetter>::
DisplayFaceHighlightColor (const Settings& settings, 
			   PropertySetter propertySetter,
			   typename DisplayElement::FocusContext focus,
			   bool useZPos, double zPos) : 

    DisplayElementPropertyFocus<PropertySetter> (
	settings, propertySetter, focus, useZPos, zPos)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    ViewNumber::Enum viewNumber = this->GetViewNumber ();
    ViewSettings& vs = this->m_settings.GetViewSettings (viewNumber);
    if (this->m_focus == DisplayElement::FOCUS)
    {
	glColor (this->m_settings.GetHighlightColor (
		     this->GetViewNumber (), highlightColorIndex));
    }
    else
    {
        QColor color =  QColor::fromRgbF (0, 0, 0, vs.GetContextAlpha ());
	glColor (color);
    }
    (displayEdges (this->m_settings, this->GetViewNumber (), 
                   this->m_propertySetter.Is2D (),
                   this->m_focus,
                   this->m_useZPos, this->m_zPos)) (f);
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    operator () (of->GetFace ());
}

// DisplayFaceBodyScalarColor
// ======================================================================

template<typename PropertySetter>
DisplayFaceBodyScalarColor<PropertySetter>::
DisplayFaceBodyScalarColor (
    const Settings& settings, bool is2D,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	settings, 
        PropertySetter (settings, view, is2D), focus, useZPos, zPos)
{
}

template<typename PropertySetter>
DisplayFaceBodyScalarColor<PropertySetter>::
DisplayFaceBodyScalarColor (
    const Settings& settings, PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	settings, propertySetter, focus, useZPos, zPos) 
{
}



template<typename PropertySetter>
void DisplayFaceBodyScalarColor<PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    glNormal (of->GetNormal ());
    if (this->m_propertySetter.Is2D ())
    {
	bool useColor;
	setColorOrTexture (of, &useColor);
	if (useColor)
	    glDisable (GL_TEXTURE_1D);

	// write to the stencil buffer 1s for the concave polygon
	glStencilFunc (GL_NEVER, 0, 0);
	glStencilOp (GL_INVERT, GL_KEEP, GL_KEEP);
	(DisplayFaceTriangleFan (
            this->m_settings, this->GetViewNumber (), this->Is2D ())) (of);
	
	// write to the color buffer only if the stencil bit is 1
	// and set the stencil bit to 0.
	glStencilFunc (GL_NOTEQUAL, 0, 1);
	glStencilOp (GL_KEEP, GL_KEEP, GL_ZERO);
        boost::shared_ptr<Body> body = of->GetAdjacentBody ().GetBody ();
	G3D::AABox box = body->GetBoundingBox ();
	DisplayBox (G3D::Rect2D::xyxy (box.low ().xy (), box.high ().xy ()));

	if (useColor)
	    glEnable (GL_TEXTURE_1D);
    }
    else
    {
	bool useColor;
	setColorOrTexture (of, &useColor);
	if (useColor)
	    glDisable (GL_TEXTURE_1D);
	//if (of->GetId () == 4)
	//{
	//glPushAttrib (GL_POLYGON_BIT);
	//glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	DisplayFaceTriangleFan (
            this->m_settings, this->GetViewNumber (), this->Is2D ()) (of);
	    
        //glPopAttrib ();
        //}
	if (useColor)
	    glEnable (GL_TEXTURE_1D);
    }
}


template<typename PropertySetter>
void DisplayFaceBodyScalarColor<PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    ViewNumber::Enum viewNumber = this->GetViewNumber ();
    ViewSettings& vs = this->m_settings.GetViewSettings (viewNumber);
    if (this->m_focus == DisplayElement::FOCUS && ! vs.IsScalarContext ())
    {
	if (this->m_propertySetter.GetBodyOrFaceScalar () == 
	    FaceScalar::DMP_COLOR)
	{
	    glColor (of->GetColor (
			 this->m_settings.GetHighlightColor (
			     this->GetViewNumber (),
			     HighlightNumber::H0)));
	    this->m_propertySetter ();
	}
	else
	{
	    boost::shared_ptr<Body> body = of->GetAdjacentBody ().GetBody ();
	    BodyScalar::Enum property = BodyScalar::FromSizeT (
		this->m_propertySetter.GetBodyOrFaceScalar ());
	    glColor (Qt::white);
	    bool deduced;
	    bool exists = body->HasScalarValue (property, &deduced);
	    if (exists && 
		(! deduced || 
		 (deduced && 
		  this->m_settings.IsMissingPropertyShown (property))))
	    {
		*useColor = false;
		this->m_propertySetter (body);

	    }
	    else
		this->m_propertySetter ();
	}
    }
    else
	glColor (QColor::fromRgbF(0, 0, 0, vs.GetContextAlpha ()));
}


// DisplayFaceDmpColor
// ======================================================================
template<QRgb faceColor, typename PropertySetter>
DisplayFaceDmpColor<faceColor, PropertySetter>::
DisplayFaceDmpColor (
    const Settings& settings, 
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	settings, 
        // setter not used: use a default value for is2D = true
        PropertySetter (settings, view, true), 
        focus, useZPos, zPos)
{
}

template<QRgb faceColor, typename PropertySetter>
DisplayFaceDmpColor<faceColor, PropertySetter>::
DisplayFaceDmpColor (
    const Settings& settings, 
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	settings, propertySetter, focus, useZPos, zPos) 
{
}

template<QRgb faceColor, typename PropertySetter>
void DisplayFaceDmpColor<faceColor, PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    // ~/Documents/swansea-phd/foam/straight_6x2q_2/straightq_2_12_0001.dmp
    // has a standalone face (#13) that is not closed.
    if (of->IsClosed ())
    {	
	glNormal (of->GetNormal ());
	displayNoNormal (of->GetFace ());
    }
}

template<QRgb faceColor, typename PropertySetter>
void DisplayFaceDmpColor<faceColor, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    // ~/Documents/swansea-phd/foam/straight_6x2q_2/straightq_2_12_0001.dmp
    // has a standalone face (#13) that is not closed.
    if (f->IsClosed ())
    {	
	glNormal (f->GetNormal ());
	displayNoNormal (f);
    }
}


template<QRgb faceColor, typename PropertySetter>
void DisplayFaceDmpColor<faceColor, PropertySetter>::
displayNoNormal (const boost::shared_ptr<Face>& f)
{
    glColor (f->GetColor (this->m_settings.GetHighlightColor (
			       this->GetViewNumber (),
			       HighlightNumber::H0)));
    (DisplayFaceTriangleFan (this->m_settings, this->GetViewNumber (),
                             this->Is2D ())) (f);
}


// DisplayFaceLineStrip
// ======================================================================
void DisplayFaceLineStrip::operator() (
    const boost::shared_ptr<OrientedFace>& of)
{
    operator() (of->GetFace ());
}

void DisplayFaceLineStrip::operator() (const boost::shared_ptr<Face>& f)
{
    glBegin (GL_LINE_STRIP);
    const vector<boost::shared_ptr<OrientedEdge> >& v =
	f->GetOrientedEdges ();
    for_each (v.begin (), v.end (), DisplayOrientedEdgeVertices);
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
    glEnd ();
}


// DisplayFaceEdges
// ======================================================================

template<typename displayEdge>
DisplayFaceEdges<displayEdge>::
DisplayFaceEdges (
    const Settings& widget, ViewNumber::Enum viewNumber, bool is2D,
    FocusContext focus, bool useZPos, double zPos) :
    
    DisplayElementFocus (widget, viewNumber, is2D, focus, useZPos, zPos)
{
}

template<typename displayEdge>
void DisplayFaceEdges<displayEdge>::
operator() (const boost::shared_ptr<OrientedFace>  f)
{
    operator() (f->GetFace ());
}

template<typename displayEdge>
void DisplayFaceEdges<displayEdge>::
operator () (const boost::shared_ptr<Face>  f)
{
    const vector< boost::shared_ptr<OrientedEdge> >& v = 
	f->GetOrientedEdges ();
    displayEdge display(m_settings, this->GetViewNumber (), m_is2D, m_focus, 
                        m_useZPos, m_zPos);
    for (size_t i = 0; i < v.size (); i++)
    {
	boost::shared_ptr<OrientedEdge> oe = v[i];
	display (oe);
    }
}



// Template instantiations
// ======================================================================

// DisplayFaceHighlightColor
// ======================================================================

template class DisplayFaceHighlightColor<(HighlightNumber::Enum)0, 
                                         DisplayFaceEdges<DisplayEdgePropertyColor<(DisplayElement::TessellationEdgesDisplay)0> >, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<(HighlightNumber::Enum)0, 
                                         DisplayFaceEdges<DisplayEdgePropertyColor<(DisplayElement::TessellationEdgesDisplay)1> >, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegmentQuadric, DisplayArrowHeadQuadric, true> >, 
    SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegmentLine, DisplaySegmentArrow1, true> >, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<(HighlightNumber::Enum)0, DisplayFaceEdges<DisplayEdge>, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterTextureCoordinate>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H1, DisplayFaceLineStrip, SetterTextureCoordinate>;


// DisplayFaceBodyScalarColor
// ======================================================================

template class DisplayFaceBodyScalarColor<SetterTextureCoordinate>;
template class DisplayFaceBodyScalarColor<SetterVertexAttribute>;
template class DisplayFaceBodyScalarColor<SetterDeformation>;
template class DisplayFaceBodyScalarColor<SetterNop>;
template class DisplayFaceBodyScalarColor<SetterVelocity>;


// DisplayFaceDmpColor
// ======================================================================
template class DisplayFaceDmpColor<0xff000000, SetterTextureCoordinate>;

// DisplayFaceEdges
// ======================================================================

template class DisplayFaceEdges<
    DisplayEdgeTorus <DisplaySegmentLine, DisplaySegmentArrow1, true> >;
template class DisplayFaceEdges<
    DisplayEdgeTorus<DisplaySegmentQuadric, DisplayArrowHeadQuadric, true> >;
template class DisplayFaceEdges<
    DisplayEdgePropertyColor<DisplayElement::DISPLAY_TESSELLATION_EDGES> >;
template class DisplayFaceEdges<
    DisplayEdgePropertyColor<DisplayElement::DONT_DISPLAY_TESSELLATION_EDGES> >;
