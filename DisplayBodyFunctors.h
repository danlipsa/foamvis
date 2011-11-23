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

class DisplaySegment;
class Body;
class BodyAlongTime;
class BodySelector;
class StripIterator;
class StripIteratorPoint;
class ContextSegment;
class FocusTextureSegment;
class FocusColorSegment;
class Segment;

/**
 * Functor used to display a body
 */
template <typename PropertySetter = SetterTextureCoordinate>
class DisplayBodyBase : public DisplayElementProperty<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayBodyBase (const GLWidget& widget, const FoamProperties& fp,
		     const BodySelector& bodySelector, 
		     PropertySetter propertySetter,
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
     */
    virtual void display (const boost::shared_ptr<Body>& b,
			  typename DisplayElement::FocusContext fc)
    {
	static_cast<void> (b);
	static_cast<void> (fc);
    }
    void beginContext ()
    {
	glEnable (GL_BLEND);
	glDepthMask (GL_FALSE);
    }
    void endContext ()
    {
	glDepthMask (GL_TRUE);
	glDisable (GL_BLEND);
    }
    const BodySelector& m_bodySelector;
};


class DisplayBodyDeformation : public DisplayBodyBase<>
{
public:
    /**
     * Constructor
     */
    DisplayBodyDeformation (
	const GLWidget& widget, const FoamProperties& fp,
	const BodySelector& bodySelector,
	bool useZPos = false, double zPos = 0);

protected:
    /**
     * Displays the center of a body (bubble)
     */
    virtual void display (const boost::shared_ptr<Body>& b, FocusContext fc);
};


/**
 * Functor that displays the center of a bubble
 */
class DisplayBodyCenter : public DisplayBodyBase<>
{
public:
    /**
     * Constructor
     */
    DisplayBodyCenter (
	const GLWidget& widget, const FoamProperties& fp,
	const BodySelector& bodySelector,
	bool useZPos = false, double zPos = 0);

protected:
    /**
     * Displays the center of a body (bubble)
     */
    virtual void display (const boost::shared_ptr<Body>& b, FocusContext fc);
};


/**
 * Displays a body going through all its faces
 */
template<typename displayFace, typename PropertySetter = SetterTextureCoordinate>
class DisplayBody : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayBody (
	const GLWidget& widget, const FoamProperties& fp,
	const BodySelector& bodySelector,
	typename DisplayElement::ContextType 
	contextDisplay = DisplayElement::USER_DEFINED_CONTEXT,
	ViewNumber::Enum view = ViewNumber::VIEW0,
	bool useZPos = false, double zPos = 0);

    DisplayBody (
	const GLWidget& widget, const FoamProperties& fp,
	const BodySelector& bodySelector,
	PropertySetter setter,
	typename DisplayElement::ContextType 
	    contextDisplay = DisplayElement::USER_DEFINED_CONTEXT,
	bool useZPos = false, double zPos = 0);
			

protected:
    /**
     * Displays a body going through all its faces
     */
    virtual void display (
	const boost::shared_ptr<Body>& b, 
	typename DisplayElement::FocusContext bodyFc);

private:
    typename DisplayElement::ContextType m_contextDisplay;
};


/**
 * Displays the center path for a certain body id
 *
 * @todo Use Simulation::GetBodyPropertyValue instead of
 * StripIterator functions
 */
template<typename PropertySetter = SetterTextureCoordinate,
	 typename DisplayEdge = DisplaySegment>
class DisplayCenterPath : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayCenterPath (
	const GLWidget& widget, const FoamProperties& fp, ViewNumber::Enum view, 
	const BodySelector& bodySelector,
	bool useTimeDisplacement = false, 
	double timeDisplacement = 0,
	boost::shared_ptr<ofstream> output = boost::shared_ptr<ofstream>());

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
    void valueStep (
	const StripIteratorPoint& beforeBegin,
	const StripIteratorPoint& begin,
	const StripIteratorPoint& end,
	const StripIteratorPoint& afterEnd);

    void halfValueStep (
	const StripIteratorPoint& p, const Segment& segment);

    void displaySegments ();

    G3D::Vector3 getPoint (StripIteratorPoint p) const;

    void storeFocusSegment (double value, const Segment& segment);
    void storeFocusSegment (const QColor& color, const Segment& segment);

    void storeContextSegment (const QColor& color, const Segment& segment);

    void displayContextSegment (
	const boost::shared_ptr<ContextSegment>& contextSegment);

    void displayFocusTextureSegment (
	const boost::shared_ptr<FocusTextureSegment>& segment);
    void displayFocusColorSegment (
	const boost::shared_ptr<FocusColorSegment>& segment);

private:
    DisplayEdge m_displaySegment;
    vector< boost::shared_ptr<FocusTextureSegment> > m_focusTextureSegments;
    vector< boost::shared_ptr<FocusColorSegment> > m_focusColorSegments;
    vector< boost::shared_ptr<ContextSegment> > m_contextSegments;
    boost::shared_ptr<ofstream> m_output;
    size_t m_index;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
