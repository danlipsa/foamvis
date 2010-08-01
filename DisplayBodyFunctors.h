/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Functors to display a body
 */

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__

#include "GLWidget.h"
#include "DebugStream.h"
#include "Foam.h"
#include "StripIterator.h"
#include "Enums.h"


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
    DisplayBodyBase (const GLWidget& widget) : DisplayElement (widget)
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
        if (m_widget.IsDisplayedBody (b))
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
    DisplayBodyCenter (GLWidget& widget) : DisplayBodyBase (widget) {}
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
    DisplayBody (const GLWidget& widget,
		 ContextDisplay contextDisplay = TRANSPARENT_CONTEXT) : 
	DisplayBodyBase (widget), m_contextDisplay (contextDisplay)
    {}
protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (boost::shared_ptr<Body> b, FocusContext bodyFc)
    {
	if (bodyFc == CONTEXT && m_contextDisplay == INVISIBLE_CONTEXT)
	    return;
	vector<boost::shared_ptr<OrientedFace> > v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (), displayFace(m_widget, bodyFc));
    }
private:
    ContextDisplay m_contextDisplay;
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
		       CenterPathColor::Enum centerPathColor) : 
	m_widget (widget),
	m_centerPathColor (centerPathColor)
    {
    }
    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId)
    {
	const BodyAlongTime& bat = m_widget.GetBodyAlongTime (bodyId);
	StripIterator it = bat.GetStripIterator (m_widget.GetFoamAlongTime ());
	glBegin(GL_LINES);
	it.ForEachSegment (
	    boost::bind (&DisplayCenterPath::speedRangeStep, this, _1, _2));
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
    void speedRangeStep (
	const StripIterator::StripPoint& p,
	const StripIterator::StripPoint& prev)
    {
	float colorByValue = StripIterator::GetColorByValue (
	    m_centerPathColor, p, prev);
	QColor color = m_widget.MapScalar (colorByValue);
	m_widget.qglColor (color);
	glVertex (prev.m_point);
	glVertex (p.m_point);
    }

private:
    /**
     * Where to display the center path
     */
    GLWidget& m_widget;
    CenterPathColor::Enum m_centerPathColor;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
