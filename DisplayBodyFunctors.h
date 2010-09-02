/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Functors to display a body
 */

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__

#include "BodySelector.h"
#include "DebugStream.h"
#include "Enums.h"
#include "Foam.h"
#include "GLWidget.h"
#include "StripIterator.h"

/**
 * Functor used to display a body
 */
class DisplayBodyBase : public DisplayElement
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    DisplayBodyBase (const GLWidget& widget, const BodySelector& bodySelector) : 
	DisplayElement (widget), m_bodySelector (bodySelector)
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
        bool focus = m_bodySelector (b->GetId (), m_glWidget.GetTimeStep ());
	beginFocusContext (focus);
	display (b, focus ? FOCUS : CONTEXT);
	endFocusContext (focus);
    }

protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext fc)
    {
	static_cast<void> (b);
	static_cast<void> (fc);
    }
    void beginFocusContext (bool focus)
    {
	if (focus)
	    glDisable (GL_BLEND);
	else
	    glDepthMask (GL_FALSE);
    }
    void endFocusContext (bool focus)
    {
	if (focus)
	    glEnable (GL_BLEND);
	else
	    glDepthMask (GL_TRUE);
    }
    const BodySelector& m_bodySelector;
};

/**
 * Functor that displays the center of a bubble
 */
class DisplayBodyCenter : public DisplayBodyBase
{
public:
    /**
     * Constructor
     * @param widget where to display the center of the bubble
     */
    DisplayBodyCenter (GLWidget& widget, const BodySelector& bodySelector):
	DisplayBodyBase (widget, bodySelector) 
    {}
protected:
    /**
     * Displays the center of a body (bubble)
     * @param b body to display the center of
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext fc)
    {
	if (fc == FOCUS)
	{
	    glBegin(GL_POINTS);
	    G3D::Vector3 v = b->GetCenter ();
	    glVertex(v);
	    glEnd ();
	}
    }
};


/**
 * Displays a body going through all its faces
 */
template<typename displayFace>
class DisplayBody : public DisplayBodyBase
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    DisplayBody (const GLWidget& widget, const BodySelector& bodySelector,
		 ContextDisplay contextDisplay = TRANSPARENT_CONTEXT,
		 BodyProperty::Enum bodyProperty = BodyProperty::NONE) : 
	DisplayBodyBase (widget, bodySelector),
	m_contextDisplay (contextDisplay),
	m_bodyProperty (bodyProperty)
    {}
protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext bodyFc)
    {
	if (bodyFc == CONTEXT &&
	    m_contextDisplay == INVISIBLE_CONTEXT)
	    return;
	vector<boost::shared_ptr<OrientedFace> > v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (),
		  displayFace(m_glWidget, bodyFc, m_bodyProperty));
    }
private:
    ContextDisplay m_contextDisplay;
    BodyProperty::Enum m_bodyProperty;
};


/**
 * Displays the center path for a certain body id
 * @todo Display the center path only if there is at least a segment in 
 * focus in it.
 */
class DisplayCenterPath : public DisplayBodyBase
{
public:
    /**
     * Constructor
     * @param widget where to display the center path
     */
    DisplayCenterPath (GLWidget& widget,
		       BodyProperty::Enum bodyProperty, 
		       const BodySelector& bodySelector) : 
	DisplayBodyBase (widget, bodySelector),
	m_glWidget (widget),
	m_bodyProperty (bodyProperty)
    {
    }
    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId)
    {
	const BodyAlongTime& bat = m_glWidget.GetBodyAlongTime (bodyId);
	StripIterator it = bat.GetStripIterator (m_glWidget.GetFoamAlongTime ());
	if ( (m_bodyProperty >= BodyProperty::VELOCITY_BEGIN &&
	      m_bodyProperty < BodyProperty::VELOCITY_END) ||
	     m_bodyProperty == BodyProperty::NONE)
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::speedStep,
			     this, _1, _2));
	else
	{
	    it.ForEachSegment (
		boost::bind (&DisplayCenterPath::valueStep,
			     this, _1, _2));
	}
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
    void speedStep (
	const StripIterator::Point& p,
	const StripIterator::Point& prev)
    {
	bool focus = m_bodySelector (prev.m_body->GetId (), prev.m_timeStep);
	beginFocusContext (focus);
	segment (
	    focusContextColor (
		focus,
		m_glWidget.MapScalar (
		    StripIterator::GetPropertyValue (m_bodyProperty, p, prev))),
	    prev.m_point, p.m_point);
	endFocusContext (focus);
    }

    void valueStep (
	const StripIterator::Point& p,
	const StripIterator::Point& prev)
    {
	G3D::Vector3 middle = (prev.m_point + p.m_point) / 2;
	bool focus = m_bodySelector (prev.m_body->GetId (), prev.m_timeStep);
	beginFocusContext (focus);
	segment (
	    focusContextColor (
		focus,
		StripIterator::ExistsPropertyValue (m_bodyProperty, prev) ? 
		m_glWidget.MapScalar (
		    StripIterator::GetPropertyValue (m_bodyProperty, prev)) : 
		m_glWidget.GetNotAvailableCenterPathColor ()), 
	    prev.m_point, middle);
	endFocusContext (focus);

	focus = m_bodySelector (p.m_body->GetId (), p.m_timeStep);
	beginFocusContext (focus);
	segment (
	    focusContextColor (
		focus,
		StripIterator::ExistsPropertyValue (m_bodyProperty, p) ? 
		m_glWidget.MapScalar (
		    StripIterator::GetPropertyValue (m_bodyProperty, p)) : 
		m_glWidget.GetNotAvailableCenterPathColor ()), 
	    middle, p.m_point);
	endFocusContext (focus);
    }

    void beginFocusContext (bool focus)
    {
	DisplayBodyBase::beginFocusContext (focus);
	glBegin(GL_LINES);
    }
    void endFocusContext (bool focus)
    {
	glEnd ();
	DisplayBodyBase::endFocusContext (focus);
    }

    QColor focusContextColor (bool focus, const QColor& color)
    {
	if (focus)
	    return color;
	else
	{
	    QColor returnColor (Qt::black);
	    returnColor.setAlphaF (m_glWidget.GetContextAlpha ());
	    return returnColor;
	}
    }
    void segment (const QColor& color, G3D::Vector3 begin, G3D::Vector3 end)
    {
	m_glWidget.qglColor (color);
	glVertex (begin);
	glVertex (end);
    }

private:
    /**
     * Where to display the center path
     */
    GLWidget& m_glWidget;
    BodyProperty::Enum m_bodyProperty;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
