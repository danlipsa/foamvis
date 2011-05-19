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
    if (this->m_glWidget.IsDisplayedFace (m_count))
    {
	display (of);
	if (m_count == this->m_glWidget.GetSelectedFaceIndex ())
	    cdbg << "face " << m_count << ": " << *of << endl;
    }
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

    glNormal (of->GetNormal ());
    bool useColor;
    setColorOrTexture (of, &useColor);
    if (useColor)
	glDisable (GL_TEXTURE_1D);
    // clear stencil buffer
    // disable writing into the color buffer
    // set stencil function to GL_ALWAYS and stencil operation to GL_INVERT    
    (DisplayFaceTriangleFan (this->m_glWidget)) (of);
    if (useColor)
	glEnable (GL_TEXTURE_1D);
}

template<typename PropertySetter>
void DisplayFaceBodyPropertyColor<PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    boost::shared_ptr<Body> body;
    boost::shared_ptr<ViewSettings> vs;
    size_t bodyId = 0;
    if (! of->IsStandalone ())
    {
	body = of->GetBodyPartOf ().GetBody ();
	vs = this->m_glWidget.GetViewSettings (
	    this->m_propertySetter.GetViewNumber ());
	bodyId = body->GetId ();
    }
    if (! of->IsStandalone () && this->m_glWidget.IsBodyStationaryMarked () &&
	bodyId == vs->GetStationaryBodyId ())
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     HighlightNumber::H0));
	BodyProperty::Enum property = 
	    this->m_propertySetter.GetBodyProperty ();
	bool deduced;
	bool exists = 
	    body->ExistsPropertyValue (property, &deduced);
	if (exists && 
	    (! deduced || 
	     (deduced && this->m_glWidget.IsZeroedPressureShown ())))
	    this->m_propertySetter (body);
    }
    else if (! of->IsStandalone () && vs->IsBodyContext (bodyId))
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     HighlightNumber::H1));
	BodyProperty::Enum property = 
	    this->m_propertySetter.GetBodyProperty ();
	bool deduced;
	bool exists = 
	    body->ExistsPropertyValue (property, &deduced);
	if (exists && 
	    (! deduced || 
	     (deduced && this->m_glWidget.IsZeroedPressureShown ())))
	    this->m_propertySetter (body);
    }
    else if (this->m_focus == DisplayElement::FOCUS)
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
    
    DisplayFaceHighlightColor<HighlightNumber::H0, displaySameEdges, PropertySetter> (
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

    DisplayFaceHighlightColor<HighlightNumber::H0, displaySameEdges, PropertySetter> (
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
	const ViewSettings& vs = *this->m_glWidget.GetViewSettings (
	    this->m_propertySetter.GetViewNumber ());
	stationaryOrContext = bodyId == vs.GetStationaryBodyId () || 
	    vs.IsBodyContext (bodyId);
    }
    glColor ((this->m_focus == DisplayElement::FOCUS || stationaryOrContext) ?
	     QColor (faceColor) :
	     QColor::fromRgbF (0, 0, 0, this->m_glWidget.GetContextAlpha ()));
    (displaySameEdges (this->m_glWidget, this->m_focus, 
		       this->m_useZPos, this->m_zPos)) (of);
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
