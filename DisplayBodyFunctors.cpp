/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "Body.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "DebugStream.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"


struct ContextSegment : public Segment
{
    ContextSegment () : Segment ()
    {
    }
    
    ContextSegment (const QColor& color, const Segment& segment) :
	Segment (segment), m_color (color)
    {
    }
    QColor m_color;
};

struct FocusTextureSegment : public Segment
{
    FocusTextureSegment () : Segment (), m_textureCoordinate (0)
    {
    }

    FocusTextureSegment (GLfloat textureCoordinate,
		  const Segment& segment) :
	Segment (segment), m_textureCoordinate (textureCoordinate)
    {
    }
    GLfloat m_textureCoordinate;
};

struct FocusColorSegment : public Segment
{
    FocusColorSegment () : Segment ()
    {
    }

    FocusColorSegment (const QColor& color,
		       const Segment& segment) :
	Segment (segment), m_color (color)
    {
    }
    QColor m_color;
};




// DisplayBodyBase
// ======================================================================

template <typename PropertySetter>
DisplayBodyBase<PropertySetter>::
DisplayBodyBase (const GLWidget& widget,
		 const BodySelector& bodySelector,
		 PropertySetter propertySetter,
		 BodyProperty::Enum property, bool useZPos, double zPos) :

    DisplayElementProperty<PropertySetter> (
	widget, propertySetter, property, useZPos, zPos),
    m_bodySelector (bodySelector)
{
}

template <typename PropertySetter>
void DisplayBodyBase<PropertySetter>::
operator () (boost::shared_ptr<Body> b)
{
    bool focus = m_bodySelector (
	b->GetId (), this->m_glWidget.GetTimeStep ());
    if (focus)
	display (b, DisplayElement::FOCUS);
    else
    {
	beginContext ();
	display (b, DisplayElement::CONTEXT);
	endContext ();
    }
}

// DisplayBodyCenter
// ======================================================================

DisplayBodyCenter::DisplayBodyCenter (
    const GLWidget& widget, const BodySelector& bodySelector,
    bool useZPos, double zPos):

    DisplayBodyBase<> (widget, bodySelector, TexCoordSetter(widget),
		       BodyProperty::NONE, useZPos, zPos)
{}


void DisplayBodyCenter::display (boost::shared_ptr<Body> b, FocusContext fc)
{
    if (fc == FOCUS)
    {
	glBegin(GL_POINTS);
	G3D::Vector3 v = b->GetCenter ();
	::glVertex(G3D::Vector3 (v.xy (), m_zPos));
	glEnd ();
    }
}

// DisplayBody
// ======================================================================

template<typename displayFace, typename PropertySetter>
DisplayBody<displayFace, PropertySetter>::
DisplayBody (
    const GLWidget& widget, const BodySelector& bodySelector,
    typename DisplayElement::ContextType
    contextDisplay, BodyProperty::Enum property, bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
	widget, bodySelector, PropertySetter (widget), property,
	useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}

template<typename displayFace, typename PropertySetter>
DisplayBody<displayFace, PropertySetter>::
DisplayBody (
    const GLWidget& widget, const BodySelector& bodySelector,
    PropertySetter setter,
    BodyProperty::Enum property,
    typename DisplayElement::ContextType contextDisplay,
    bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
	widget, bodySelector, setter, property, useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}


template<typename displayFace, typename PropertySetter>
void DisplayBody<displayFace, PropertySetter>::
display (
    boost::shared_ptr<Body> b, typename DisplayElement::FocusContext bodyFc)
{
    if (bodyFc == DisplayElement::CONTEXT &&
	m_contextDisplay == DisplayElement::INVISIBLE_CONTEXT)
	return;
    vector<boost::shared_ptr<OrientedFace> > v = b->GetOrientedFaces ();
    for_each (
	v.begin (), v.end (),
	displayFace(
	    this->m_glWidget,
	    this->m_propertySetter, bodyFc, this->m_property,
	    this->m_useZPos, this->m_zPos));
}


// DisplayCenterPath
// ======================================================================

template<typename PropertySetter, typename DisplaySegment>
DisplayCenterPath<PropertySetter, DisplaySegment>::
DisplayCenterPath (
    const GLWidget& widget,
    BodyProperty::Enum property,
    const BodySelector& bodySelector,
    bool useTimeDisplacement,
    double timeDisplacement,
    boost::shared_ptr<ofstream> output) :

    DisplayBodyBase<PropertySetter> (
	widget, bodySelector, PropertySetter (widget), property,
	useTimeDisplacement, timeDisplacement),
    m_displaySegment (this->m_glWidget.GetQuadricObject (),
		      this->m_glWidget.GetEdgeRadius ()),
    m_output (output),
    m_index (0)
{
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
operator () (size_t bodyId)
{
    m_focusTextureSegments.resize (0);
    m_focusColorSegments.resize (0);
    m_contextSegments.resize (0);
    const BodyAlongTime& bat = this->m_glWidget.GetBodyAlongTime (bodyId);
    StripIterator it = bat.GetStripIterator (
	this->m_glWidget.GetFoamAlongTime ());
    it.ForEachSegment (
	boost::bind (&DisplayCenterPath::valueStep, this, _1, _2, _3, _4));
    displaySegments ();
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
valueStep (
    const StripIteratorPoint& beforeBegin,
    const StripIteratorPoint& begin,
    const StripIteratorPoint& end,
    const StripIteratorPoint& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    G3D::Vector3 pointBegin = getPoint (begin);
    G3D::Vector3 pointEnd = getPoint (end);
    G3D::Vector3 middle = (pointBegin + pointEnd) / 2;
    halfValueStep (
	begin,
	Segment (beforeBegin.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END :
		 SegmentPerpendicularEnd::END,
		 getPoint (beforeBegin), pointBegin, middle, G3D::Vector3 ()));
    halfValueStep (
	end,
	Segment (
	    afterEnd.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END :
	    SegmentPerpendicularEnd::BEGIN,
	    G3D::Vector3 (), middle, pointEnd, getPoint (afterEnd)));
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
halfValueStep (const StripIteratorPoint& p, const Segment& segment)
{
    bool focus = this->m_bodySelector (p.m_body->GetId (), p.m_timeStep);
    if (focus)
    {
	if (p.m_body->ExistsPropertyValue (this->m_property))
	    storeFocusSegment (
		p.m_body->GetPropertyValue (this->m_property), segment);
	else
	    storeFocusSegment (
		this->m_glWidget.NOT_AVAILABLE_CENTER_PATH_COLOR, segment);
    }
    else
	storeContextSegment (
	    this->m_glWidget.GetCenterPathContextColor (), segment);
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displaySegments ()
{
    for_each (
	m_focusTextureSegments.begin (), m_focusTextureSegments.end (),
	boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
		     displayFocusTextureSegment, this, _1));
    if (! this->m_glWidget.IsContextHidden () && m_contextSegments.size () > 0)
    {
	glDisable (GL_TEXTURE_1D);
	DisplayBodyBase<PropertySetter>::beginContext ();
	for_each (
	    m_contextSegments.begin (), m_contextSegments.end (),
	    boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
			 displayContextSegment, this, _1));
	DisplayBodyBase<PropertySetter>::endContext ();
	glEnable (GL_TEXTURE_1D);
    }
    if (m_focusColorSegments.size () > 0)
    {
	glDisable (GL_TEXTURE_1D);
	for_each (
	    m_focusColorSegments.begin (), m_focusColorSegments.end (),
	    boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
			 displayFocusColorSegment, this, _1));
	glEnable (GL_TEXTURE_1D);
    }
}

template<typename PropertySetter, typename DisplaySegment>
G3D::Vector3 DisplayCenterPath<PropertySetter, DisplaySegment>::
getPoint (StripIteratorPoint p) const
{
    if (this->m_useZPos)
    {
	G3D::Vector3 v = p.m_point;
	v.z = p.m_timeStep * this->m_zPos;
	return v;
    }
    else
	return p.m_point;
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeFocusSegment (double value, const Segment& segment)
{
    double textureCoordinate = 
	this->m_glWidget.GetColorBarModel ().TexCoord (value);
    boost::shared_ptr<FocusTextureSegment> fs = 
	boost::make_shared<FocusTextureSegment> (textureCoordinate, segment);
    m_focusTextureSegments.push_back (fs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeFocusSegment (const QColor& color, const Segment& segment)
{
    boost::shared_ptr<FocusColorSegment> fs = 
	boost::make_shared<FocusColorSegment> (color, segment);
    m_focusColorSegments.push_back (fs);
}



template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeContextSegment (
    const QColor& color, const Segment& segment)
{
    boost::shared_ptr<ContextSegment> cs = boost::make_shared<ContextSegment> (
	color, segment);
    m_contextSegments.push_back (cs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayContextSegment (
    const boost::shared_ptr<ContextSegment>& contextSegment)
{
    glColor (contextSegment->m_color);
    m_displaySegment (*contextSegment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusTextureSegment (
    const boost::shared_ptr<FocusTextureSegment>& segment)
{
    glColor (Qt::white);
    glTexCoord1f (segment->m_textureCoordinate);
    m_displaySegment (*segment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusColorSegment (const boost::shared_ptr<FocusColorSegment>& segment)
{
    glColor (segment->m_color);
    m_displaySegment (*segment);
}



// Template instantiations
// ======================================================================

// DisplayBodyBase
// ======================================================================

template class DisplayBodyBase<VertexAttributeSetter>;
template class DisplayBodyBase<TexCoordSetter>;

// DisplayBody
// ======================================================================

template class DisplayBody<
    DisplayFace<
	DisplayEdges<
	    DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >,
	TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFace<
	DisplayEdges<
	    DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >,
	TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFace<
	DisplayEdges<DisplayEdgeTorusClipped>,
	TexCoordSetter>,
    TexCoordSetter>;
template class DisplayBody<
    DisplayFace<DisplaySameEdges, TexCoordSetter>, TexCoordSetter>;

template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, VertexAttributeSetter>,
    VertexAttributeSetter>;

template class DisplayBody<
    DisplayFace<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, VertexAttributeSetter>,
    VertexAttributeSetter>;


// DisplayCenterPath
// ======================================================================

template class DisplayCenterPath<TexCoordSetter, DisplayEdgeTube>;
template class DisplayCenterPath<TexCoordSetter, DisplayEdgeQuadric>;
template class DisplayCenterPath<TexCoordSetter, DisplayEdge>;
