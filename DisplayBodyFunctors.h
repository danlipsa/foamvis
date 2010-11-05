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
 * @todo Display the center path only if there is at least a segment in 
 * focus in it.
 */
template<typename PropertySetter = TexCoordSetter,
	 typename DisplayEdgeType = DisplayEdge>
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
	m_displayEdge (this->m_glWidget.GetQuadricObject (), 
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
    }

    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId)
    {
	
	const BodyAlongTime& bat = this->m_glWidget.GetBodyAlongTime (bodyId);
	StripIterator it = bat.GetStripIterator (
	    this->m_glWidget.GetFoamAlongTime ());
	if ( (this->m_bodyProperty >= BodyProperty::VELOCITY_BEGIN &&
	      this->m_bodyProperty < BodyProperty::VELOCITY_END) ||
	     this->m_bodyProperty == BodyProperty::NONE)
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::speedStep,
			     this, _1, _2));
	else
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::valueStep,
			     this, _1, _2));
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

    void speedStep (
	const StripIterator::Point& p,
	const StripIterator::Point& prev)
    {
	bool focus = this->m_bodySelector (
	    prev.m_body->GetId (), prev.m_timeStep);
	if (focus && this->m_bodyProperty != BodyProperty::NONE)
	    texturedSegment (
		StripIterator::GetPropertyValue (this->m_bodyProperty, p, prev),
		getPoint (prev, this->m_useZPos, this->m_zPos), 
		getPoint (p, this->m_useZPos, this->m_zPos));
	else
	{
	    QColor color = (this->m_bodyProperty == BodyProperty::NONE) ? 
		this->m_glWidget.GetCenterPathNotAvailableColor () :
		this->m_glWidget.GetCenterPathContextColor ();
	    coloredSegment (color, false, 
			    getPoint (prev, this->m_useZPos, this->m_zPos), 
			    getPoint (p, this->m_useZPos, this->m_zPos));
	}
    }

    void valueStep (
	const StripIterator::Point& p,
	const StripIterator::Point& prev)
    {
	G3D::Vector3 middle = (getPoint (prev, this->m_useZPos, this->m_zPos) + 
			       getPoint (p, this->m_useZPos, this->m_zPos)) / 2;
	halfValueStep (prev, middle, false);
	halfValueStep (p, middle, true);
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
	    texturedSegment (
		StripIterator::GetPropertyValue (
		    this->m_bodyProperty, p), point, middle);
	else
	{
	    QColor color = (focus) ? 
		this->m_glWidget.GetCenterPathNotAvailableColor () :
		this->m_glWidget.GetCenterPathContextColor ();		
	    coloredSegment (color, focus, point, middle);
	}
    }


    QColor focusContextColor (bool focus, const QColor& color)
    {
	if (focus)
	    return color;
	else
	    return this->m_glWidget.GetCenterPathContextColor ();
    }
    void segment (const QColor& color, G3D::Vector3 begin, G3D::Vector3 end)
    {
	glColor (color);
	m_displayEdge (begin, end);
    }

    void segment (float texCoord, G3D::Vector3 begin, G3D::Vector3 end)
    {
	glTexCoord1f (texCoord);
	m_displayEdge (begin, end);
    }
    
    void texturedSegment (double value, G3D::Vector3 begin, G3D::Vector3 end)
    {
	double segmentTexCoord = this->m_glWidget.TexCoord (value);
	DisplayBodyBase<PropertySetter>::beginFocusContext (true);
	segment (segmentTexCoord, begin, end);
	DisplayBodyBase<PropertySetter>::endFocusContext (true);
    }

    void coloredSegment (const QColor& color, bool focus,
			 G3D::Vector3 begin, G3D::Vector3 end)
    {
	glDisable (GL_TEXTURE_1D);
	DisplayBodyBase<PropertySetter>::beginFocusContext (focus);
	segment (color, begin, end);
	DisplayBodyBase<PropertySetter>::endFocusContext (focus);
	glEnable (GL_TEXTURE_1D);
    }
private:
    DisplayEdgeType m_displayEdge;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
