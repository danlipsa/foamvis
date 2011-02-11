/**
 * @file   DisplayFaceFunctors.h
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Implementation for functors that display a face
 */

#include "DebugStream.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "OrientedFace.h"

// DisplayFace
// ======================================================================

template <typename displayEdges, typename PropertySetter>
DisplayFace<displayEdges, PropertySetter>::
DisplayFace (const GLWidget& widget,
	     typename DisplayElement::FocusContext focus,
	     BodyProperty::Enum property, bool useZPos, double zPos) : 
    
    DisplayElementPropertyFocus<PropertySetter> (
	widget, PropertySetter (widget), property, focus, 
	useZPos, zPos), 
    m_count(0)
{
}

template <typename displayEdges, typename PropertySetter>
DisplayFace<displayEdges, PropertySetter>::
DisplayFace (const GLWidget& widget,
	     PropertySetter propertySetter,
	     typename DisplayElement::FocusContext focus,
	     BodyProperty::Enum property, bool useZPos, double zPos) : 
    DisplayElementPropertyFocus<PropertySetter> (
	widget, propertySetter, property, focus, useZPos, zPos), 
    m_count(0)
{
}

template <typename displayEdges, typename PropertySetter>
void DisplayFace<displayEdges, PropertySetter>::
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

template <typename displayEdges, typename PropertySetter>
void DisplayFace<displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    boost::shared_ptr<OrientedFace> of = 
	boost::make_shared<OrientedFace>(f, false);
    operator () (of);
}

template <typename displayEdges, typename PropertySetter>
void DisplayFace<displayEdges, PropertySetter>::
display (const boost::shared_ptr<OrientedFace>& of)
{
    if (this->m_focus == DisplayElement::FOCUS)
    {
	glColor (G3D::Color4 (Color::BLACK, 1.));
    }
    else
	glColor (
	    G3D::Color4 (Color::BLACK, this->m_glWidget.GetContextAlpha ()));
    (displayEdges (this->m_glWidget, this->m_focus, 
		   this->m_useZPos, this->m_zPos)) (of);
}

// DisplayFaceWithColor
// ======================================================================

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceWithColor<displaySameEdges, PropertySetter>::
DisplayFaceWithColor (
    const GLWidget& widget,
    typename DisplayElement::FocusContext focus, BodyProperty::Enum property, 
    bool useZPos, double zPos) : 
    
    DisplayFace<displaySameEdges, PropertySetter> (
	widget, PropertySetter (widget), focus, property, useZPos, zPos)
{
}

template<typename displaySameEdges, typename PropertySetter>
DisplayFaceWithColor<displaySameEdges, PropertySetter>::
DisplayFaceWithColor (
    const GLWidget& widget,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    BodyProperty::Enum property, bool useZPos, double zPos) : 

    DisplayFace<displaySameEdges, PropertySetter> (
	widget, propertySetter, focus, property, useZPos, zPos) 
{
}

template<typename displaySameEdges, typename PropertySetter>
void DisplayFaceWithColor<displaySameEdges, PropertySetter>::
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

    /*
    // prepare the stencil
    glClear (GL_STENCIL_BUFFER_BIT);
    glDrawBuffer (GL_NONE);
    glEnable (GL_STENCIL_TEST);
    glStencilOp (GL_KEEP, GL_KEEP, GL_INVERT);
    (displaySameEdges (this->m_glWidget)) (of);
	

    // draw the concave polygon
    glDrawBuffer (GL_FRONT);
    glStencilFunc (GL_GREATER, 0, 0xff);

    glNormal (of->GetNormal ());
    bool useColor;
    setColorOrTexture (of, &useColor);
    if (useColor)
	glDisable (GL_TEXTURE_1D);
    (displaySameEdges (this->m_glWidget)) (of);
    if (useColor)
	glEnable (GL_TEXTURE_1D);
   
    glDisable (GL_STENCIL_TEST);
    */
}

template<typename displaySameEdges, typename PropertySetter>
void DisplayFaceWithColor<displaySameEdges, PropertySetter>::
setColorOrTexture (const boost::shared_ptr<OrientedFace>& of, 
		   bool* useColor)
{
    *useColor = true;
    if (this->m_focus == DisplayElement::FOCUS)
    {
	if (this->m_property == BodyProperty::NONE)
	    glColor (Color::GetValue(of->GetColor ()));
	else
	{
	    size_t bodyId = of->GetBodyPartOf ().GetBodyId ();
	    QColor color;
	    const FoamAlongTime& foamAlongTime = 
		this->m_glWidget.GetFoamAlongTime ();
	    if (foamAlongTime.ExistsBodyProperty (
		    this->m_property, bodyId, 
		    this->m_glWidget.GetTimeStep ()))
	    {
		double value = foamAlongTime.GetBodyPropertyValue (
		    this->m_property, bodyId, 
		    this->m_glWidget.GetTimeStep ());
		this->m_propertySetter (value);
		*useColor = false;
	    }
	    else
		glColor (this->m_glWidget.GetNotAvailableFaceColor ());
	}
    }
    else
	glColor (G3D::Color4 (Color::GetValue(Color::BLACK),
			      this->m_glWidget.GetContextAlpha ()));
}

// Template instantiations
// ======================================================================

// DisplayFace
// ======================================================================

template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true> >, 
    TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> >, TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorusClipped>, TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    TexCoordSetter>;
template class DisplayFace<DisplaySameEdges, TexCoordSetter>;
template class DisplayFace<DisplaySameTriangles, TexCoordSetter>;
template class DisplayFace<DisplaySameTriangles, VertexAttributeSetter>;
template class DisplayFace<DisplaySameEdges, VertexAttributeSetter>;

// DisplayFaceWithColor
// ======================================================================

template class DisplayFaceWithColor<DisplaySameEdges, TexCoordSetter>;
template class DisplayFaceWithColor<DisplaySameTriangles, TexCoordSetter>;
template class DisplayFaceWithColor<DisplaySameTriangles, VertexAttributeSetter>;
template class DisplayFaceWithColor<DisplaySameEdges, VertexAttributeSetter>;
