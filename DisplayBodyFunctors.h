/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Functors to display a body
 */

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__

#include "DisplayElement.h"
#include "Enums.h"

class DisplayEdge;
class Body;
class BodyAlongTime;
class BodySelector;
class StripIterator;
class StripIteratorPoint;
class ContextSegment;
class FocusSegment;
class Segment;

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
		     BodyProperty::Enum property = BodyProperty::NONE,
		     bool useZPos = false, double zPos = 0);

    /**
     * Functor used  to display a body. Uses  transparencey to display
     * the context.
     *
     * See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
     * Antialiasing, Fog and Polygon Offset page 293
     *
     * @param b the body to be displayed
     */
    void operator () (boost::shared_ptr<Body> b);

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
	BodyProperty::Enum property = BodyProperty::NONE,
	bool useZPos = false, double zPos = 0);

    DisplayBody (
	const GLWidget& widget, const BodySelector& bodySelector,
	PropertySetter setter,
	BodyProperty::Enum property = BodyProperty::NONE,
	typename DisplayElement::ContextType 
	contextDisplay = DisplayElement::TRANSPARENT_CONTEXT,
	bool useZPos = false, double zPos = 0);

protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (
	boost::shared_ptr<Body> b, typename DisplayElement::FocusContext bodyFc);

private:
    typename DisplayElement::ContextType m_contextDisplay;
};


/**
 * Displays the center path for a certain body id
 *
 * @todo Use FoamAlongTime::GetBodyPropertyValue instead of
 * StripIterator functions
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
		       BodyProperty::Enum property, 
		       const BodySelector& bodySelector,
		       bool useTimeDisplacement = false, 
		       double timeDisplacement = 0);

    DisplayCenterPath (const GLWidget& widget,
		       PropertySetter propertySetter,
		       BodyProperty::Enum property, 
		       const BodySelector& bodySelector);

    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId);

    /**
     * Helper function which calls operator () (size_t bodyId).
     * @param p a pair original index body pointer
     */
    inline void operator () (
	const map <size_t, boost::shared_ptr<BodyAlongTime> >::value_type& p)
    {
	operator() (p.first);
    }
    

private:

    void copySegments (StripIterator& it);

    void speedStep (
	const StripIteratorPoint& beforeBegin,
	const StripIteratorPoint& begin,
	const StripIteratorPoint& end,
	const StripIteratorPoint& afterEnd);

    void valueStep (
	const StripIteratorPoint& beforeBegin,
	const StripIteratorPoint& begin,
	const StripIteratorPoint& end,
	const StripIteratorPoint& afterEnd);

    void halfValueStep (
	const StripIteratorPoint& p, const Segment& segment);

    void displaySegments ();

    G3D::Vector3 getPoint (StripIteratorPoint p) const;

    QColor focusContextColor (bool focus, const QColor& color);

    void storeFocusSegment (
	double value, const Segment& segment);

    void storeContextSegment (
	const QColor& color, bool focus, const Segment& segment);

    void displayContextSegment (
	const boost::shared_ptr<ContextSegment>& contextSegment);

    void displayFocusSegment (
	const boost::shared_ptr<FocusSegment>& focusSegment);

private:
    DisplaySegment m_displaySegment;
    vector< boost::shared_ptr<FocusSegment> > m_focusSegments;
    vector< boost::shared_ptr<ContextSegment> > m_contextSegments;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
