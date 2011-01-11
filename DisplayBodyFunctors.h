/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Functors to display a body
 */

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__

#include "Body.h"
#include "BodyAlongTime.h"
#include "BodySelector.h"
#include "DebugStream.h"
#include "DisplayElement.h"
#include "Enums.h"
#include "Foam.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "StripIterator.h"
#include "PropertySetter.h"
#include "DisplayEdgeFunctors.h"

/**
 * Functor used to display a body
 */
template <typename PropertySetter = TexCoordSetter>
class DisplayBodyBase : public DisplayElementProperty<PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    DisplayBodyBase (const GLWidget& widget, 
		     const BodySelector& bodySelector, 
		     PropertySetter propertySetter,
		     BodyProperty::Enum bodyProperty = BodyProperty::NONE,
		     bool useZPos = false, double zPos = 0) : 
	DisplayElementProperty<PropertySetter> (
	    widget, propertySetter, bodyProperty, useZPos, zPos), 
	m_bodySelector (bodySelector)
    {}

    /**
     * Functor used  to display a body. Uses  transparencey to display
     * the context.
     *
     * See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
     * Antialiasing, Fog and Polygon Offset page 293
     *
     * @param b the body to be displayed
     */
    void operator () (boost::shared_ptr<Body> b)
    {
        bool focus = m_bodySelector (b->GetId (), 
				     this->m_glWidget.GetTimeStep ());
	beginFocusContext (focus);
	display (b, focus ? DisplayElement::FOCUS : DisplayElement::CONTEXT);
	endFocusContext (focus);
    }

protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (boost::shared_ptr<Body> b,
			  typename DisplayElement::FocusContext fc)
    {
	static_cast<void> (b);
	static_cast<void> (fc);
    }
    void beginFocusContext (bool focus)
    {
	if (! focus)
	{
	    glEnable (GL_BLEND);
	    glDepthMask (GL_FALSE);
	}
    }
    void endFocusContext (bool focus)
    {
	if (! focus)
	{
	    glDepthMask (GL_TRUE);
	    glDisable (GL_BLEND);
	}
    }
    const BodySelector& m_bodySelector;
};

/**
 * Functor that displays the center of a bubble
 */
class DisplayBodyCenter : public DisplayBodyBase<>
{
public:
    /**
     * Constructor
     * @param widget where to display the center of the bubble
     */
    DisplayBodyCenter (
	const GLWidget& widget, const BodySelector& bodySelector,
	bool useZPos = false, double zPos = 0);

protected:
    /**
     * Displays the center of a body (bubble)
     * @param b body to display the center of
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext fc);
};


/**
 * Displays a body going through all its faces
 */
template<typename displayFace, typename PropertySetter = TexCoordSetter>
class DisplayBody : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    DisplayBody (
	const GLWidget& widget, const BodySelector& bodySelector,
	typename DisplayElement::ContextType 
	contextDisplay = DisplayElement::TRANSPARENT_CONTEXT,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE,
	bool useZPos = false, double zPos = 0) : 

	DisplayBodyBase<PropertySetter> (
	    widget, bodySelector, PropertySetter (widget), bodyProperty, 
	    useZPos, zPos),
	m_contextDisplay (contextDisplay)
    {}

    DisplayBody (
	const GLWidget& widget, const BodySelector& bodySelector,
	PropertySetter setter,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE,
	typename DisplayElement::ContextType 
	contextDisplay = DisplayElement::TRANSPARENT_CONTEXT,
	bool useZPos = false, double zPos = 0) : 

	DisplayBodyBase<PropertySetter> (
	    widget, bodySelector, setter, bodyProperty, useZPos, zPos),
	m_contextDisplay (contextDisplay)
    {}


protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (
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
		this->m_propertySetter, bodyFc, this->m_bodyProperty, 
		this->m_useZPos, this->m_zPos));
    }
private:
    typename DisplayElement::ContextType m_contextDisplay;
};


/**
 * Displays the center path for a certain body id
 * @todo Use FoamAlongTime::GetBodyPropertyValue instead of StripIterator functions
 */
template<typename PropertySetter = TexCoordSetter,
	 typename DisplaySegment = DisplayEdge>
class DisplayCenterPath : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     * @param widget where to display the center path
     */
    DisplayCenterPath (const GLWidget& widget,
		       BodyProperty::Enum bodyProperty, 
		       const BodySelector& bodySelector,
		       bool useTimeDisplacement = false, 
		       double timeDisplacement = 0) : 
	DisplayBodyBase<PropertySetter> (
	    widget, bodySelector, PropertySetter (widget), bodyProperty,
	    useTimeDisplacement, timeDisplacement),
	m_displaySegment (this->m_glWidget.GetQuadricObject (), 
		       this->m_glWidget.GetEdgeRadius ())

    {
    }

    DisplayCenterPath (const GLWidget& widget,
		       PropertySetter propertySetter,
		       BodyProperty::Enum bodyProperty, 
		       const BodySelector& bodySelector) : 
	DisplayBodyBase<PropertySetter> (
	    widget, bodySelector, propertySetter, bodyProperty, false, 0)
    {
	size_t timeSteps = this->m_glWidget.GetFoamAlongTime ().GetTimeSteps ();
	m_focusSegments.reserve (timeSteps - 1);
	m_contextSegments.reserve (timeSteps - 1);
    }

    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId)
    {
	m_focusSegments.resize (0);
	m_contextSegments.resize (0);
	const BodyAlongTime& bat = this->m_glWidget.GetBodyAlongTime (bodyId);
	StripIterator it = bat.GetStripIterator (
	    this->m_glWidget.GetFoamAlongTime ());
	copySegments (it);
	displaySegments ();
    }

    /**
     * Helper function which calls operator () (size_t bodyId).
     * @param p a pair original index body pointer
     */
    inline void operator () (const BodiesAlongTime::BodyMap::value_type& p)
    {
	operator() (p.first);
    }

private:
    struct ContextSegment
    {
	ContextSegment () :
	    m_focus (false)
	{
	}
	ContextSegment (const QColor& color, bool focus, 
			const G3D::Vector3& begin,
			const G3D::Vector3& end) :
	    m_color (color), m_focus (focus), m_begin (begin), m_end (end)
	{
	}
	QColor m_color;
	bool m_focus;
	G3D::Vector3 m_begin;
	G3D::Vector3 m_end;
    };

    struct FocusSegment
    {
	FocusSegment () :
	    m_textureCoordinate (0)
	{
	}
	FocusSegment (GLfloat textureCoordinate, const G3D::Vector3& begin,
			 const G3D::Vector3& end) :
	    m_textureCoordinate (textureCoordinate), m_begin (begin), m_end (end)
	{
	}
	GLfloat m_textureCoordinate;
	G3D::Vector3 m_begin;
	G3D::Vector3 m_end;	
    };

private:

    void copySegments (StripIterator& it)
    {
	if ( (this->m_bodyProperty >= BodyProperty::VELOCITY_BEGIN &&
	      this->m_bodyProperty < BodyProperty::VELOCITY_END) ||
	     this->m_bodyProperty == BodyProperty::NONE)
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::speedStep,
			     this, _1, _2, _3, _4));
	else
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::valueStep,
			     this, _1, _2, _3, _4));
    }

    void speedStep (
	const StripIterator::Point& beforeBegin,
	const StripIterator::Point& begin,
	const StripIterator::Point& end,
	const StripIterator::Point& afterEnd)
    {
	static_cast<void>(beforeBegin);
	static_cast<void>(afterEnd);
	bool focus = this->m_bodySelector (
	    begin.m_body->GetId (), begin.m_timeStep);
	if (focus && this->m_bodyProperty != BodyProperty::NONE)
	    storeFocusSegment (
		StripIterator::GetVelocityValue (
		    this->m_bodyProperty, end, begin),
		getPoint (begin, this->m_useZPos, this->m_zPos), 
		getPoint (end, this->m_useZPos, this->m_zPos));
	else
	{
	    QColor color = (this->m_bodyProperty == BodyProperty::NONE) ? 
		this->m_glWidget.GetCenterPathNotAvailableColor () :
		this->m_glWidget.GetCenterPathContextColor ();
	    storeContextSegment (color, false, 
			    getPoint (begin, this->m_useZPos, this->m_zPos), 
			    getPoint (end, this->m_useZPos, this->m_zPos));
	}
    }

    void valueStep (
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

    void halfValueStep (const StripIterator::Point& p, G3D::Vector3 middle,
			bool swapPoints)
    {
	G3D::Vector3 point = getPoint (p, this->m_useZPos, this->m_zPos);
	if (swapPoints)
	    swap (point, middle);
	bool focus = this->m_bodySelector (p.m_body->GetId (), p.m_timeStep);
	if (focus && StripIterator::ExistsPropertyValue (
		this->m_bodyProperty, p))
	    storeFocusSegment (
		StripIterator::GetPropertyValue (
		    this->m_bodyProperty, p), point, middle);
	else
	{
	    QColor color = (focus) ? 
		this->m_glWidget.GetCenterPathNotAvailableColor () :
		this->m_glWidget.GetCenterPathContextColor ();		
	    storeContextSegment (color, focus, point, middle);
	}
    }

    void displaySegments ()
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

    G3D::Vector3 getPoint (StripIterator::Point p, bool useTimeDisplacement,
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

    QColor focusContextColor (bool focus, const QColor& color)
    {
	if (focus)
	    return color;
	else
	    return this->m_glWidget.GetCenterPathContextColor ();
    }
    void displayContextSegment (const ContextSegment& coloredSegment)
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

    void displayFocusSegment (const FocusSegment& texturedSegment)
    {
	DisplayBodyBase<PropertySetter>::beginFocusContext (true);
	glTexCoord1f (texturedSegment.m_textureCoordinate);
	m_displaySegment (texturedSegment.m_begin, texturedSegment.m_end);
	DisplayBodyBase<PropertySetter>::endFocusContext (true);
    }

    void storeFocusSegment (
	double value, G3D::Vector3 begin, G3D::Vector3 end)
    {
	double textureCoordinate = this->m_glWidget.TexCoord (value);
	m_focusSegments.push_back (
	    FocusSegment (textureCoordinate, begin, end));
    }

    void storeContextSegment (const QColor& color, bool focus,
			      G3D::Vector3 begin, G3D::Vector3 end)
    {
	m_contextSegments.push_back (
	    ContextSegment (color, focus, begin, end));
    }



private:
    DisplaySegment m_displaySegment;
    vector<FocusSegment> m_focusSegments;
    vector<ContextSegment> m_contextSegments;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
