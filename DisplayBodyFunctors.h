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
        if (m_bodySelector (b->GetId (), m_glWidget.GetTimeStep ()))
	{
	    glDisable (GL_BLEND);
	    display (b, FOCUS);
	    glEnable (GL_BLEND);
	}
	else
	{
	    glDepthMask (GL_FALSE);
	    display (b, CONTEXT);
	    glDepthMask (GL_TRUE);
	}
    }

protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext fc) = 0;
private:
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
 * Displays the center path for a certain body
 */
class DisplayCenterPath
{
public:
    /**
     * Constructor
     * @param widget where to display the center path
     */
    DisplayCenterPath (GLWidget& widget,
		       BodyProperty::Enum bodyProperty) : 
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
	glBegin(GL_LINES);
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
	glEnd ();
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
	segment (StripIterator::GetPropertyValue (m_bodyProperty, p, prev),
		 prev.m_point, p.m_point);
    }

    void valueStep (
	const StripIterator::Point& p,
	const StripIterator::Point& prev)
    {
	G3D::Vector3 middle = (prev.m_point + p.m_point) / 2;
	if (StripIterator::ExistsPropertyValue (m_bodyProperty, prev))
	    segment (StripIterator::GetPropertyValue (
			 m_bodyProperty, prev), prev.m_point, middle);
	else
	    segment (m_glWidget.GetNotAvailableCenterPathColor (), 
		     prev.m_point, middle);

	if (StripIterator::ExistsPropertyValue (m_bodyProperty, p))
	    segment (StripIterator::GetPropertyValue (
			 m_bodyProperty, p), middle, p.m_point);
	else
	    segment (m_glWidget.GetNotAvailableCenterPathColor (), 
		     middle, p.m_point);
    }

    void segment (double value, G3D::Vector3 begin, G3D::Vector3 end)
    {
	segment (m_glWidget.MapScalar (value), begin, end);
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
