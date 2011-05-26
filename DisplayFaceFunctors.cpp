/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Implementation for functors that display a face
 */

#include "Body.h"
#include "DebugStream.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "OrientedFace.h"
#include "ViewSettings.h"

// DisplayFaceHighlightColor
// ======================================================================

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceHighlightColor<highlightColorIndex, 
			  displayEdges, PropertySetter>::
DisplayFaceHighlightColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus,
    ViewNumber::Enum view, bool useZPos, double zPos) : 
    
    DisplayElementPropertyFocus<PropertySetter> (
	widget, PropertySetter (widget, view), focus, useZPos, zPos), 
    m_count(0)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceHighlightColor<highlightColorIndex, 
			  displayEdges, PropertySetter>::
DisplayFaceHighlightColor (const GLWidget& widget,
			   PropertySetter propertySetter,
			   typename DisplayElement::FocusContext focus,
			   bool useZPos, double zPos) : 

    DisplayElementPropertyFocus<PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos), 
    m_count(0)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    display (of);
    m_count++;
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    boost::shared_ptr<OrientedFace> of = 
	boost::make_shared<OrientedFace>(f, false);
    operator () (of);
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    if (this->m_focus == DisplayElement::FOCUS)
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     highlightColorIndex));
    }
    else
	glColor (QColor::fromRgbF (
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
    (displayEdges (this->m_glWidget, this->m_focus, 
		   this->m_useZPos, this->m_zPos)) (of);
}

// DisplayFaceBodyPropertyColor
// ======================================================================

template<typename PropertySetter>
DisplayFaceBodyPropertyColor<PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<HighlightNumber::H0, 
			      DisplayFaceTriangleFan, PropertySetter> (
	widget, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<typename PropertySetter>
DisplayFaceBodyPropertyColor<PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<HighlightNumber::H0, 
			      DisplayFaceTriangleFan, PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos) 
{
}

template<typename PropertySetter>
void DisplayFaceBodyPropertyColor<PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    if (this->m_glWidget.GetFoamAlongTime ().Is2D ())
    {
	glNormal (of->GetNormal ());
	bool useColor;
	setColorOrTexture (of, &useColor);
	if (useColor)
	    glDisable (GL_TEXTURE_1D);

	// write to the stencil buffer 1s for the concave polygon
	glStencilFunc (GL_NEVER, 0, 0);
	glStencilOp (GL_INVERT, GL_KEEP, GL_KEEP);
	(DisplayFaceTriangleFan (this->m_glWidget)) (of);
	
	// write to the color buffer only if the stencil bit is 1
	// and set the stencil bit to 0.
	glStencilFunc (GL_NOTEQUAL, 0, 1);
	glStencilOp (GL_KEEP, GL_KEEP, GL_ZERO);
	boost::shared_ptr<Body> body = of->GetBodyPartOf ().GetBody ();
	G3D::AABox box = body->GetBoundingBox ();
	DisplayBox (G3D::Rect2D::xyxy (box.low ().xy (), box.high ().xy ()));

	if (useColor)
	    glEnable (GL_TEXTURE_1D);
    }
    else
    {
	glNormal (of->GetNormal ());
	bool useColor;
	setColorOrTexture (of, &useColor);
	if (useColor)
	    glDisable (GL_TEXTURE_1D);
	(DisplayFaceTriangleFan (this->m_glWidget)) (of);
	if (useColor)
	    glEnable (GL_TEXTURE_1D);
    }
}


template<typename PropertySetter>
void DisplayFaceBodyPropertyColor<PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    boost::shared_ptr<Body> body;
    size_t bodyId = 0;
    if (! of->IsStandalone ())
    {
	body = of->GetBodyPartOf ().GetBody ();
	bodyId = body->GetId ();
    }
    if (this->m_focus == DisplayElement::FOCUS)
    {
	if (this->m_propertySetter.GetBodyProperty () == BodyProperty::NONE)
	{
	    glColor (of->GetColor (this->m_glWidget.GetHighlightColor (
				       this->m_propertySetter.GetViewNumber (),
				       HighlightNumber::H0)));
	    this->m_propertySetter ();
	}
	else
	{
	    BodyProperty::Enum property = 
		this->m_propertySetter.GetBodyProperty ();
	    bool deduced;
	    bool exists = 
		body->ExistsPropertyValue (property, &deduced);
	    if (exists && 
		(! deduced || 
		 (deduced && this->m_glWidget.IsZeroedPressureShown ())))
	    {
		glColor (Qt::white);
		*useColor = false;
		this->m_propertySetter (body);

	    }
	    else
	    {
		glColor (this->m_glWidget.GetHighlightColor (
			     this->m_propertySetter.GetViewNumber (),
			     HighlightNumber::H1));
		this->m_propertySetter ();
	    }
	}
    }
    else
	glColor (QColor::fromRgbF(
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
}


// DisplayFaceColor
// ======================================================================
template<QRgb faceColor, typename displaySameEdges, typename PropertySetter>
DisplayFaceColor<faceColor, displaySameEdges, PropertySetter>::
DisplayFaceColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<HighlightNumber::H0, 
			      displaySameEdges, PropertySetter> (
	widget, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<QRgb faceColor, typename displaySameEdges, typename PropertySetter>
DisplayFaceColor<faceColor, displaySameEdges, PropertySetter>::
DisplayFaceColor (
    const GLWidget& widget,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<HighlightNumber::H0, 
			      displaySameEdges, PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos) 
{
}

template<QRgb faceColor, typename displaySameEdges, typename PropertySetter>
void DisplayFaceColor<faceColor, displaySameEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    bool stationaryOrContext;
    if (of->IsStandalone ())
	stationaryOrContext = false;
    else
    {
	boost::shared_ptr<Body> body = of->GetBodyPartOf ().GetBody ();
	size_t bodyId = body->GetId ();
	const ViewSettings& vs = this->m_glWidget.GetViewSettings (
	    this->m_propertySetter.GetViewNumber ());
	stationaryOrContext = ((bodyId == vs.GetStationaryBodyId ()) || 
			       vs.IsContextDisplayBody (bodyId));
    }
    if (! stationaryOrContext)
    {
	glColor (this->m_focus == DisplayElement::FOCUS ?
		 QColor (faceColor) :
		 QColor::fromRgbF (0, 0, 0, 
				   this->m_glWidget.GetContextAlpha ()));
	(displaySameEdges (this->m_glWidget, this->m_focus, 
			   this->m_useZPos, this->m_zPos)) (of);
    }
}



// Template instantiations
// ======================================================================

// DisplayFaceHighlightColor
// ======================================================================

template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegmentQuadric, DisplaySegmentArrowQuadric, true> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow, true> >, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorusClipped>, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgePropertyColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgePropertyColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterValueVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterValueVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H1, DisplayFaceLineStrip, SetterValueTextureCoordinate>;


// DisplayFaceBodyPropertyColor
// ======================================================================

template class DisplayFaceBodyPropertyColor<SetterValueTextureCoordinate>;
template class DisplayFaceBodyPropertyColor<SetterValueVertexAttribute>;

// DisplayFaceColor
// ======================================================================
template class DisplayFaceColor<0xff000000, DisplayFaceLineStrip, SetterValueTextureCoordinate>;
