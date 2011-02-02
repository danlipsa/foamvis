/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "Body.h"
#include "BodySelector.h"
#include "DebugStream.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"


struct ContextSegment : public Segment
{
    ContextSegment () : Segment (), m_focus (false)
    {
    }

    ContextSegment (const QColor& color, bool focus, 
		    const Segment& segment) :
	Segment (segment), m_color (color), m_focus (focus)
    {
    }
    QColor m_color;
    bool m_focus;
};

struct FocusSegment : public Segment
{
    FocusSegment () : Segment (), m_textureCoordinate (0)
    {
    }

    FocusSegment (GLfloat textureCoordinate,
		  const Segment& segment) :
	Segment (segment), m_textureCoordinate (textureCoordinate)
    {
    }
    GLfloat m_textureCoordinate;
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
    bool focus = m_bodySelector (b->GetId (), 
				 this->m_glWidget.GetTimeStep ());
    beginFocusContext (focus);
    display (b, focus ? DisplayElement::FOCUS : DisplayElement::CONTEXT);
    endFocusContext (focus);
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
    m_focusSegments.resize (0);
    m_contextSegments.resize (0);
    const BodyAlongTime& bat = this->m_glWidget.GetBodyAlongTime (bodyId);
    StripIterator it = bat.GetStripIterator (
	this->m_glWidget.GetFoamAlongTime ());
    copySegments (it);
    displaySegments ();
    ++m_index;
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
copySegments (StripIterator& it)
{
    if ( (this->m_property >= BodyProperty::VELOCITY_BEGIN &&
	  this->m_property < BodyProperty::VELOCITY_END) ||
	 this->m_property == BodyProperty::NONE)
	it.ForEachSegment (
	    boost::bind (&DisplayCenterPath::speedStep,
			 this, _1, _2, _3, _4));
    else
	it.ForEachSegment (
	    boost::bind (&DisplayCenterPath::valueStep,
			 this, _1, _2, _3, _4));
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
speedStep (
    const StripIteratorPoint& beforeBegin,
    const StripIteratorPoint& begin,
    const StripIteratorPoint& end,
    const StripIteratorPoint& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    bool focus = this->m_bodySelector (
	begin.m_body->GetId (), begin.m_timeStep);
    if (focus && this->m_property != BodyProperty::NONE)
	storeFocusSegment (
	    begin.m_body->GetPropertyValue (this->m_property),
	    Segment (
		StripIterator::GetSegmentPerpendicularEnd (begin, end),
		getPoint (beforeBegin), 
		getPoint (begin), 
		getPoint (end),
		getPoint (afterEnd)));
    else
    {
	QColor color = (this->m_property == BodyProperty::NONE) ? 
	    this->m_glWidget.GetCenterPathNotAvailableColor () :
	    this->m_glWidget.GetCenterPathContextColor ();
	storeContextSegment (
	    color, false, 
	    Segment (
		StripIterator::GetSegmentPerpendicularEnd (begin, end),
		getPoint (beforeBegin), 
		getPoint (begin), 
		getPoint (end),
		getPoint (afterEnd)));
    }
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
    halfValueStep (begin,
		   Segment (
		       beforeBegin.IsEmpty () ? 
		       SegmentPerpendicularEnd::BEGIN_END : 
		       SegmentPerpendicularEnd::END,
		       getPoint (beforeBegin), pointBegin, 
		       middle, G3D::Vector3 ()));
    halfValueStep (end,
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
    if (focus && p.m_body->ExistsPropertyValue (this->m_property))
	storeFocusSegment (
	    p.m_body->GetPropertyValue (this->m_property), segment);
    else
    {
	QColor color = (focus) ? 
	    this->m_glWidget.GetCenterPathNotAvailableColor () :
	    this->m_glWidget.GetCenterPathContextColor ();		
	storeContextSegment (color, focus, segment);
    }
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displaySegments ()
{
    if (m_focusSegments.size () > 0 && m_output.get () != 0)
    {
	G3D::Vector3 begin = m_focusSegments[0]->m_begin;
	(*m_output) << m_index << " " 
		    << begin.x << " " << begin.y << " " << begin.z << endl;
    }
    for_each (
	m_focusSegments.begin (), m_focusSegments.end (),
	boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
		     displayFocusSegment, this, _1));
    if (! this->m_glWidget.OnlyPathsWithSelectionShown () ||
	m_focusSegments.size () != 0)
	for_each (
	    m_contextSegments.begin (), m_contextSegments.end (),
	    boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
			 displayContextSegment, this, _1));
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
QColor DisplayCenterPath<PropertySetter, DisplaySegment>::
focusContextColor (bool focus, const QColor& color)
{
    if (focus)
	return color;
    else
	return this->m_glWidget.GetCenterPathContextColor ();
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeFocusSegment (double value, const Segment& segment)
{
    double textureCoordinate = this->m_glWidget.TexCoord (value);
    boost::shared_ptr<FocusSegment> fs = boost::make_shared<FocusSegment> (
	textureCoordinate, segment);
    m_focusSegments.push_back (fs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeContextSegment (
    const QColor& color, bool focus, const Segment& segment)
{
    boost::shared_ptr<ContextSegment> cs = boost::make_shared<ContextSegment> (
	color, focus, segment);
    m_contextSegments.push_back (cs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayContextSegment (
    const boost::shared_ptr<ContextSegment>& contextSegment)
{
    glDisable (GL_TEXTURE_1D);
    DisplayBodyBase<PropertySetter>::beginFocusContext (contextSegment->m_focus);
    glColor (contextSegment->m_color);
    m_displaySegment (*contextSegment);
    DisplayBodyBase<PropertySetter>::endFocusContext (contextSegment->m_focus);
    glEnable (GL_TEXTURE_1D);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusSegment (const boost::shared_ptr<FocusSegment>& segment)
{
    DisplayBodyBase<PropertySetter>::beginFocusContext (true);
    glTexCoord1f (segment->m_textureCoordinate);
    m_displaySegment (*segment);
    DisplayBodyBase<PropertySetter>::endFocusContext (true);
    G3D::Vector3 end = segment->m_end;
    if (m_output.get () != 0)
	(*m_output) << m_index << " " 
		    << end.x << " " << end.y << " " << end.z << endl;
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
    DisplayFace<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;

template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, VertexAttributeSetter>, 
    VertexAttributeSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, VertexAttributeSetter>, 
    VertexAttributeSetter>;


// DisplayCenterPath
// ======================================================================

template class DisplayCenterPath<TexCoordSetter, DisplayEdgeTube>;
template class DisplayCenterPath<TexCoordSetter, DisplayEdge>;
