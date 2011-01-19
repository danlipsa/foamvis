/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "Body.h"
#include "BodySelector.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"

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
	glVertex(G3D::Vector3 (v.xy (), m_zPos));
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
DisplayCenterPath (const GLWidget& widget,
		   BodyProperty::Enum property, 
		   const BodySelector& bodySelector,
		   bool useTimeDisplacement, 
		   double timeDisplacement) : 

    DisplayBodyBase<PropertySetter> (
	widget, bodySelector, PropertySetter (widget), property,
	useTimeDisplacement, timeDisplacement),
    m_displaySegment (this->m_glWidget.GetQuadricObject (), 
		      this->m_glWidget.GetEdgeRadius ())
    
{
}

template<typename PropertySetter, typename DisplaySegment>
DisplayCenterPath<PropertySetter, DisplaySegment>::
DisplayCenterPath (const GLWidget& widget,
		   PropertySetter propertySetter,
		   BodyProperty::Enum property, 
		   const BodySelector& bodySelector) : 
    DisplayBodyBase<PropertySetter> (
	widget, bodySelector, propertySetter, property, false, 0)
{
    size_t timeSteps = this->m_glWidget.GetFoamAlongTime ().GetTimeSteps ();
    m_focusSegments.reserve (timeSteps - 1);
    m_contextSegments.reserve (timeSteps - 1);
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
    const StripIterator::Point& beforeBegin,
    const StripIterator::Point& begin,
    const StripIterator::Point& end,
    const StripIterator::Point& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    bool focus = this->m_bodySelector (
	begin.m_body->GetId (), begin.m_timeStep);
    if (focus && this->m_property != BodyProperty::NONE)
	storeFocusSegment (
	    StripIterator::GetVelocityValue (
		this->m_property, end, begin),
	    getPoint (begin, this->m_useZPos, this->m_zPos), 
	    getPoint (end, this->m_useZPos, this->m_zPos));
    else
    {
	QColor color = (this->m_property == BodyProperty::NONE) ? 
	    this->m_glWidget.GetCenterPathNotAvailableColor () :
	    this->m_glWidget.GetCenterPathContextColor ();
	storeContextSegment (color, false, 
			     getPoint (begin, this->m_useZPos, this->m_zPos), 
			     getPoint (end, this->m_useZPos, this->m_zPos));
    }
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
valueStep (
    const StripIterator::Point& beforeBegin,
    const StripIterator::Point& begin,
    const StripIterator::Point& end,
    const StripIterator::Point& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    G3D::Vector3 middle = (
	getPoint (begin, this->m_useZPos, this->m_zPos) + 
	getPoint (end, this->m_useZPos, this->m_zPos)) / 2;
    halfValueStep (begin, middle, false);
    halfValueStep (end, middle, true);
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
halfValueStep (const StripIterator::Point& p, G3D::Vector3 middle,
	       bool swapPoints)
{
    G3D::Vector3 point = getPoint (p, this->m_useZPos, this->m_zPos);
    if (swapPoints)
	swap (point, middle);
    bool focus = this->m_bodySelector (p.m_body->GetId (), p.m_timeStep);
    if (focus && StripIterator::ExistsPropertyValue (
	    this->m_property, p))
	storeFocusSegment (
	    StripIterator::GetPropertyValue (
		this->m_property, p), point, middle);
    else
    {
	QColor color = (focus) ? 
	    this->m_glWidget.GetCenterPathNotAvailableColor () :
	    this->m_glWidget.GetCenterPathContextColor ();		
	storeContextSegment (color, focus, point, middle);
    }
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displaySegments ()
{
    for_each (m_focusSegments.begin (),
	      m_focusSegments.end (),
	      boost::bind (
		  &DisplayCenterPath<PropertySetter, 
		  DisplaySegment>::displayFocusSegment, this, _1));
    if (! this->m_glWidget.OnlyPathsWithSelectionShown () ||
	m_focusSegments.size () != 0)
	for_each (m_contextSegments.begin (),
		  m_contextSegments.end (),
		  boost::bind (
		      &DisplayCenterPath<PropertySetter, 
		      DisplaySegment>::displayContextSegment, this, _1));
}

template<typename PropertySetter, typename DisplaySegment>
G3D::Vector3 DisplayCenterPath<PropertySetter, DisplaySegment>::
getPoint (StripIterator::Point p, bool useTimeDisplacement,
	  double timeDisplacement)
{
    if (useTimeDisplacement)
    {
	G3D::Vector3 v = p.m_point;
	v.z = p.m_timeStep * timeDisplacement;
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
storeFocusSegment (
    double value, G3D::Vector3 begin, G3D::Vector3 end)
{
    double textureCoordinate = this->m_glWidget.TexCoord (value);
    m_focusSegments.push_back (
	FocusSegment (textureCoordinate, begin, end));
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeContextSegment (const QColor& color, bool focus,
		     G3D::Vector3 begin, G3D::Vector3 end)
{
    m_contextSegments.push_back (
	ContextSegment (color, focus, begin, end));
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayContextSegment (const ContextSegment& coloredSegment)
{
    glDisable (GL_TEXTURE_1D);
    DisplayBodyBase<PropertySetter>::beginFocusContext (
	coloredSegment.m_focus);
    glColor (coloredSegment.m_color);
    m_displaySegment (coloredSegment.m_begin, coloredSegment.m_end);
    DisplayBodyBase<PropertySetter>::endFocusContext (
	coloredSegment.m_focus);
    glEnable (GL_TEXTURE_1D);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusSegment (const FocusSegment& texturedSegment)
{
    DisplayBodyBase<PropertySetter>::beginFocusContext (true);
    glTexCoord1f (texturedSegment.m_textureCoordinate);
    m_displaySegment (texturedSegment.m_begin, texturedSegment.m_end);
    DisplayBodyBase<PropertySetter>::endFocusContext (true);
}
