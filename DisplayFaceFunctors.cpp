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

// DisplayFaceHighlightColor
// ======================================================================

template <size_t highlightColorIndex,
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

template <size_t highlightColorIndex,
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

template <size_t highlightColorIndex,
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

template <size_t highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    boost::shared_ptr<OrientedFace> of = 
	boost::make_shared<OrientedFace>(f, false);
    operator () (of);
}

template <size_t highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    if (this->m_focus == DisplayElement::FOCUS)
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     HighlightNumber::Enum (highlightColorIndex)));
    }
    else
	glColor (QColor::fromRgbF (
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
    (displayEdges (this->m_glWidget, this->m_focus, 
		   this->m_useZPos, this->m_zPos)) (of);
}

// DisplayFaceBodyPropertyColor
// ======================================================================

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceBodyPropertyColor<displaySameEdges, PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<0, displaySameEdges, PropertySetter> (
	widget, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceBodyPropertyColor<displaySameEdges, PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<0, displaySameEdges, PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos) 
{
}

template<typename displaySameEdges, typename PropertySetter>
void DisplayFaceBodyPropertyColor<displaySameEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{

    glNormal (of->GetNormal ());
    bool useColor;
    setColorOrTexture (of, &useColor);
    if (useColor)
	glDisable (GL_TEXTURE_1D);
    (displaySameEdges (this->m_glWidget)) (of);
    if (useColor)
	glEnable (GL_TEXTURE_1D);
}

template<typename displaySameEdges, typename PropertySetter>
void DisplayFaceBodyPropertyColor<displaySameEdges, PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    boost::shared_ptr<Body> body = of->GetBodyPartOf ().GetBody ();
    size_t bodyId = body->GetId ();
    if (bodyId == this->m_glWidget.GetBodyStationaryId ())
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     HighlightNumber::HIGHLIGHT0));
	//this->m_propertySetter (body);
    }
    else if (this->m_glWidget.IsBodyContext (bodyId))
    {
	glColor (this->m_glWidget.GetHighlightColor (
		     this->m_propertySetter.GetViewNumber (),
		     HighlightNumber::HIGHLIGHT1));
	//this->m_propertySetter (body);
    }
    else if (this->m_focus == DisplayElement::FOCUS)
    {
	if (this->m_propertySetter.GetBodyProperty () == BodyProperty::NONE)
	{
	    glColor (of->GetColor (this->m_glWidget.GetHighlightColor (
				       this->m_propertySetter.GetViewNumber (),
				       HighlightNumber::HIGHLIGHT0)));
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
			     HighlightNumber::HIGHLIGHT1));
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
    
    DisplayFaceHighlightColor<0, displaySameEdges, PropertySetter> (
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

    DisplayFaceHighlightColor<0, displaySameEdges, PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos) 
{
}

template<QRgb faceColor, typename displaySameEdges, typename PropertySetter>
void DisplayFaceColor<faceColor, displaySameEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    boost::shared_ptr<Body> body = of->GetBodyPartOf ().GetBody ();
    size_t bodyId = body->GetId ();
    if (this->m_focus == DisplayElement::FOCUS ||
	bodyId == this->m_glWidget.GetBodyStationaryId () ||
	this->m_glWidget.IsBodyContext (bodyId))
    {
	glColor (QColor (faceColor));
    }
    else
	glColor (QColor::fromRgbF (
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
    (displaySameEdges (this->m_glWidget, this->m_focus, 
		       this->m_useZPos, this->m_zPos)) (of);
}



// Template instantiations
// ======================================================================

// DisplayFaceHighlightColor
// ======================================================================

template class DisplayFaceHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> >, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorusClipped>, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0,
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0,
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0, DisplayFaceLineStrip, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0, DisplayFaceLineStrip, SetterValueVertexAttribute>;

template class DisplayFaceHighlightColor<0, DisplayFaceTriangleFan, SetterValueTextureCoordinate>;
template class DisplayFaceHighlightColor<0, DisplayFaceTriangleFan, SetterValueVertexAttribute>;

template class DisplayFaceHighlightColor<1ul, DisplayFaceLineStrip, SetterValueTextureCoordinate>;


// DisplayFaceBodyPropertyColor
// ======================================================================

template class DisplayFaceBodyPropertyColor<DisplayFaceLineStrip, SetterValueTextureCoordinate>;
template class DisplayFaceBodyPropertyColor<DisplayFaceLineStrip, SetterValueVertexAttribute>;
template class DisplayFaceBodyPropertyColor<DisplayFaceTriangleFan, SetterValueTextureCoordinate>;
template class DisplayFaceBodyPropertyColor<DisplayFaceTriangleFan, SetterValueVertexAttribute>;

// DisplayFaceColor
// ======================================================================
template class DisplayFaceColor<0xff000000, DisplayFaceLineStrip, SetterValueTextureCoordinate>;
