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

// DisplayFaceWithHighlightColor
// ======================================================================

template <size_t highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceWithHighlightColor<highlightColorIndex, 
			      displayEdges, PropertySetter>::
DisplayFaceWithHighlightColor (const GLWidget& widget,
	     typename DisplayElement::FocusContext focus,
	     BodyProperty::Enum property, bool useZPos, double zPos) : 
    
    DisplayElementPropertyFocus<PropertySetter> (
	widget, PropertySetter (widget, property), focus, useZPos, zPos), 
    m_count(0)
{
}

template <size_t highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceWithHighlightColor<highlightColorIndex, 
			      displayEdges, PropertySetter>::
DisplayFaceWithHighlightColor (const GLWidget& widget,
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
void DisplayFaceWithHighlightColor<highlightColorIndex, 
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
void DisplayFaceWithHighlightColor<highlightColorIndex, 
				   displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    boost::shared_ptr<OrientedFace> of = 
	boost::make_shared<OrientedFace>(f, false);
    operator () (of);
}

template <size_t highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceWithHighlightColor<highlightColorIndex, 
				   displayEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    if (this->m_focus == DisplayElement::FOCUS)
    {
	glColor (this->m_glWidget.GetHighlightColor (highlightColorIndex));
    }
    else
	glColor (QColor::fromRgbF (
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
    (displayEdges (this->m_glWidget, this->m_focus, 
		   this->m_useZPos, this->m_zPos)) (of);
}

// DisplayFaceWithBodyPropertyColor
// ======================================================================

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceWithBodyPropertyColor<displaySameEdges, PropertySetter>::
DisplayFaceWithBodyPropertyColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus, BodyProperty::Enum property, 
    bool useZPos, double zPos) : 
    
    DisplayFaceWithHighlightColor<0, displaySameEdges, PropertySetter> (
	widget, PropertySetter (widget, property), 
	focus, useZPos, zPos)
{
}

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceWithBodyPropertyColor<displaySameEdges, PropertySetter>::
DisplayFaceWithBodyPropertyColor (
    const GLWidget& widget,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceWithHighlightColor<0, displaySameEdges, PropertySetter> (
	widget, propertySetter, focus, useZPos, zPos) 
{
}

template<typename displaySameEdges, typename PropertySetter>
void DisplayFaceWithBodyPropertyColor<displaySameEdges, PropertySetter>::
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
void DisplayFaceWithBodyPropertyColor<displaySameEdges, PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    if (this->m_focus == DisplayElement::FOCUS)
    {
	if (this->m_propertySetter.GetBodyProperty () == BodyProperty::NONE)
	{
	    glColor (of->GetColor (this->m_glWidget.GetHighlightColor (0)));
	    this->m_propertySetter ();
	}
	else
	{
	    boost::shared_ptr<Body> body = of->GetBodyPartOf ().GetBody ();
	    if (body->ExistsPropertyValue (
		    this->m_propertySetter.GetBodyProperty ()))
	    {
		size_t bodyId = body->GetId ();
		if (bodyId == this->m_glWidget.GetStationaryBodyId ())
		    glColor (this->m_glWidget.GetHighlightColor (0));
		else if (this->m_glWidget.IsStationaryBodyContext (bodyId))
		    glColor (this->m_glWidget.GetHighlightColor (1));
		else
		{
		    glColor (Qt::white);
		    *useColor = false;
		}
		this->m_propertySetter (body);

	    }
	    else
	    {
		glColor (this->m_glWidget.GetHighlightColor (0));
		this->m_propertySetter ();
	    }
	}
    }
    else
	glColor (QColor::fromRgbF(0, 0, 0, this->m_glWidget.GetContextAlpha ()));
}

// Template instantiations
// ======================================================================

// DisplayFaceWithHighlightColor
// ======================================================================

template class DisplayFaceWithHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> >, SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0,
    DisplayEdges<
	DisplayEdgeTorusClipped>, SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0,
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0,
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0, DisplayFaceLineStrip, SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0, DisplayFaceLineStrip, SetterValueVertexAttribute>;

template class DisplayFaceWithHighlightColor<0, DisplayFaceTriangleFan, SetterValueTextureCoordinate>;
template class DisplayFaceWithHighlightColor<0, DisplayFaceTriangleFan, SetterValueVertexAttribute>;

template class DisplayFaceWithHighlightColor<1ul, DisplayFaceLineStrip, SetterValueTextureCoordinate>;


// DisplayFaceWithBodyPropertyColor
// ======================================================================

template class DisplayFaceWithBodyPropertyColor<DisplayFaceLineStrip, SetterValueTextureCoordinate>;
template class DisplayFaceWithBodyPropertyColor<DisplayFaceLineStrip, SetterValueVertexAttribute>;
template class DisplayFaceWithBodyPropertyColor<DisplayFaceTriangleFan, SetterValueTextureCoordinate>;
template class DisplayFaceWithBodyPropertyColor<DisplayFaceTriangleFan, SetterValueVertexAttribute>;
