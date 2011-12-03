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
#include "Face.h"
#include "FoamProperties.h"
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
    const GLWidget& widget, const FoamProperties& fp,
    typename DisplayElement::FocusContext focus,
    ViewNumber::Enum view, bool useZPos, double zPos) : 
    
    DisplayElementPropertyFocus<PropertySetter> (
	widget, fp, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
DisplayFaceHighlightColor<highlightColorIndex, 
			  displayEdges, PropertySetter>::
DisplayFaceHighlightColor (const GLWidget& widget, const FoamProperties& fp,
			   PropertySetter propertySetter,
			   typename DisplayElement::FocusContext focus,
			   bool useZPos, double zPos) : 

    DisplayElementPropertyFocus<PropertySetter> (
	widget, fp, propertySetter, focus, useZPos, zPos)
{
}

template <HighlightNumber::Enum highlightColorIndex,
	  typename displayEdges, typename PropertySetter>
void DisplayFaceHighlightColor<highlightColorIndex, 
			       displayEdges, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
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
    (displayEdges (this->m_glWidget, this->m_foamProperties, this->m_focus, 
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

// DisplayFaceBodyPropertyColor
// ======================================================================

template<typename PropertySetter>
DisplayFaceBodyPropertyColor<PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,const FoamProperties& fp,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	widget, fp, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<typename PropertySetter>
DisplayFaceBodyPropertyColor<PropertySetter>::
DisplayFaceBodyPropertyColor (
    const GLWidget& widget,const FoamProperties& fp,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	widget, fp, propertySetter, focus, useZPos, zPos) 
{
}

template<typename PropertySetter>
void DisplayFaceBodyPropertyColor<PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    glNormal (of->GetNormal ());

    if (this->m_foamProperties.Is2D ())
    {
	bool useColor;
	setColorOrTexture (of, &useColor);
	if (useColor)
	    glDisable (GL_TEXTURE_1D);

	// write to the stencil buffer 1s for the concave polygon
	glStencilFunc (GL_NEVER, 0, 0);
	glStencilOp (GL_INVERT, GL_KEEP, GL_KEEP);
	(DisplayFaceTriangleFan (this->m_glWidget, this->m_foamProperties)) (of);
	
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
	(DisplayFaceTriangleFan (this->m_glWidget, this->m_foamProperties)) (of);
	    
        //glPopAttrib ();
        //}
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
    if (this->m_focus == DisplayElement::FOCUS)
    {
	if (this->m_propertySetter.GetBodyOrFaceProperty () == 
	    FaceProperty::DMP_COLOR)
	{
	    glColor (of->GetColor (
			 this->m_glWidget.GetHighlightColor (
			     this->m_propertySetter.GetViewNumber (),
			     HighlightNumber::H0)));
	    this->m_propertySetter ();
	}
	else
	{
	    boost::shared_ptr<Body> body = of->GetAdjacentBody ().GetBody ();
	    BodyProperty::Enum property = BodyProperty::FromSizeT (
		this->m_propertySetter.GetBodyOrFaceProperty ());
	    bool deduced;
	    bool exists = 
		(! body->IsConstraint ()) &&  
		body->ExistsPropertyValue (property, &deduced);
	    if (exists && 
		(! deduced || 
		 (deduced && 
		  this->m_glWidget.IsMissingPropertyShown (property))))
	    {
		glColor (Qt::white);
		*useColor = false;
		this->m_propertySetter (body);

	    }
	    else
	    {
		glColor (Qt::white);
		this->m_propertySetter ();
	    }
	}
    }
    else
	glColor (QColor::fromRgbF(
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));
}


// DisplayFaceLineStripColor
// ======================================================================
template<QRgb faceColor, typename PropertySetter>
DisplayFaceLineStripColor<faceColor, PropertySetter>::
DisplayFaceLineStripColor (
    const GLWidget& widget, const FoamProperties& fp,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceLineStrip, PropertySetter> (
	widget, fp, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<QRgb faceColor, typename PropertySetter>
DisplayFaceLineStripColor<faceColor,  PropertySetter>::
DisplayFaceLineStripColor (
    const GLWidget& widget, const FoamProperties& fp,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceLineStrip, PropertySetter> (
	widget, fp, propertySetter, focus, useZPos, zPos) 
{
}

template<QRgb faceColor, typename PropertySetter>
void DisplayFaceLineStripColor<faceColor, PropertySetter>::
operator () (const boost::shared_ptr<OrientedFace>& of)
{
    bool stationaryOrContext;
    if (of->IsStandalone ())
	stationaryOrContext = false;
    else
    {
	boost::shared_ptr<Body> body = of->GetAdjacentBody ().GetBody ();
	size_t bodyId = body->GetId ();
	const ViewSettings& vs = this->m_glWidget.GetViewSettings (
	    this->m_propertySetter.GetViewNumber ());
	stationaryOrContext = ((bodyId == vs.GetAverageAroundBodyId ()) || 
			       vs.IsContextDisplayBody (bodyId));
    }
    if (! stationaryOrContext)
    {
	operator () (of->GetFace ());
    }
}

template<QRgb faceColor, typename PropertySetter>
void DisplayFaceLineStripColor<faceColor, PropertySetter>::
operator () (const boost::shared_ptr<Face>& f)
{
    glColor (this->m_focus == DisplayElement::FOCUS ?
	     QColor (faceColor) :
	     QColor::fromRgbF (0, 0, 0, 
			       this->m_glWidget.GetContextAlpha ()));
    (DisplayFaceLineStrip (this->m_glWidget, this->m_foamProperties, 
			   this->m_focus, this->m_useZPos, this->m_zPos)) (f);
}


// DisplayFaceDmpColor
// ======================================================================
template<QRgb faceColor, typename PropertySetter>
DisplayFaceDmpColor<faceColor, PropertySetter>::
DisplayFaceDmpColor (
    const GLWidget& widget, const FoamProperties& fp,
    typename DisplayElement::FocusContext focus, ViewNumber::Enum view, 
    bool useZPos, double zPos) : 
    
    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	widget, fp, PropertySetter (widget, view), focus, useZPos, zPos)
{
}

template<QRgb faceColor, typename PropertySetter>
DisplayFaceDmpColor<faceColor, PropertySetter>::
DisplayFaceDmpColor (
    const GLWidget& widget, const FoamProperties& fp,
    PropertySetter propertySetter,
    typename DisplayElement::FocusContext focus,
    bool useZPos, double zPos) : 

    DisplayFaceHighlightColor<
    HighlightNumber::H0, 
    DisplayFaceTriangleFan, PropertySetter> (
	widget, fp, propertySetter, focus, useZPos, zPos) 
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
    glColor (f->GetColor (this->m_glWidget.GetHighlightColor (
			       this->m_propertySetter.GetViewNumber (),
			       HighlightNumber::H0)));
    (DisplayFaceTriangleFan (this->m_glWidget, this->m_foamProperties)) (f);
}



// Template instantiations
// ======================================================================

// DisplayFaceHighlightColor
// ======================================================================

template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegmentQuadric, DisplaySegmentArrowQuadric, true> >, 
    SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow1, true> >, SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgeTorusClipped>, SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgePropertyColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0,
    DisplayFaceEdges<
	DisplayEdgePropertyColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterTextureCoordinate>;
template class DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterVertexAttribute>;

template class DisplayFaceHighlightColor<HighlightNumber::H1, DisplayFaceLineStrip, SetterTextureCoordinate>;


// DisplayFaceBodyPropertyColor
// ======================================================================

template class DisplayFaceBodyPropertyColor<SetterTextureCoordinate>;
template class DisplayFaceBodyPropertyColor<SetterVertexAttribute>;
template class DisplayFaceBodyPropertyColor<SetterDeformation>;
template class DisplayFaceBodyPropertyColor<SetterNop>;
template class DisplayFaceBodyPropertyColor<SetterVelocity>;

// DisplayFaceLineStripColor
// ======================================================================
template class DisplayFaceLineStripColor<0xff000000, SetterTextureCoordinate>;

// DisplayFaceDmpColor
// ======================================================================
template class DisplayFaceDmpColor<0xff000000, SetterTextureCoordinate>;
