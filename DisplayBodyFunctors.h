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
class Simulation;

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
    DisplayBodyBase (const Settings& settings, const Foam& foam,
		     const BodySelector& bodySelector, 
		     PropertySetter propertySetter,
		     bool useZPos = false, double zPos = 0);

    static void BeginContext ();
    static void EndContext ();
    bool IsFocus (boost::shared_ptr<Body> b) const
    {
	return m_bodySelector (b);
    }

    DisplayElement::FocusContext GetFocusContext (
	boost::shared_ptr<Body> b) const
    {
	return  IsFocus (b) ? 
	    DisplayElement::FOCUS : DisplayElement::CONTEXT;
    }

private:
    const BodySelector& m_bodySelector;
};


class DisplayBodyDeformation : public DisplayBodyBase<>
{
public:
    DisplayBodyDeformation (
	const Settings& settings, ViewNumber::Enum viewNumber, 
	const Foam& foam,
	const BodySelector& bodySelector, float deformationSizeInitialRatio,
	bool useZPos = false, double zPos = 0);

    void operator () (boost::shared_ptr<Body> b);
private:
    float m_deformationSizeInitialRatio;
};


class DisplayBodyVelocity : public DisplayBodyBase<>
{
public:
    DisplayBodyVelocity (
	const Settings& settings, ViewNumber::Enum viewNumber, 
	const Foam& foam,
	const BodySelector& bodySelector, float bubbleSize, 
	float velocitySizeInitialRatio, float onePixelInObjectSpace,
	bool sameSize, bool clampingShown,
	bool useZPos = false, double zPos = 0);

    void operator () (boost::shared_ptr<Body> b);

private:
    float m_bubbleSize;
    float m_velocitySizeInitialRatio;
    float m_onePixelInObjectSpace;
    bool m_sameSize;
    bool m_clampingShown;
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
	const Settings& settings, const Foam& foam,
	const BodySelector& bodySelector,
	bool useZPos = false, double zPos = 0);

    /**
     * Displays the center of a body (bubble)
     */
    void operator () (boost::shared_ptr<Body> b);
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
	const Settings& settings, const Foam& foam,
	const BodySelector& bodySelector,
	typename DisplayElement::ContextType 
	contextDisplay = DisplayElement::USER_DEFINED_CONTEXT,
	ViewNumber::Enum view = ViewNumber::VIEW0,
	bool useZPos = false, double zPos = 0);

    DisplayBody (
	const Settings& settings, const Foam& foam,
	const BodySelector& bodySelector,
	PropertySetter setter,
	typename DisplayElement::ContextType 
	    contextDisplay = DisplayElement::USER_DEFINED_CONTEXT,
	bool useZPos = false, double zPos = 0);
			
    /**
     * Displays a body going through all its faces
     */
    void operator () (boost::shared_ptr<Body> b);

private:
    typename DisplayElement::ContextType m_contextDisplay;
};


/**
 * Displays the center path for a certain body id
 *
 * @todo Use Simulation::GetBodyScalarValue instead of
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
	const Settings& settings, const Foam& foam, ViewNumber::Enum view, 
	const BodySelector& bodySelector, GLUquadricObj* quadric, 
	const Simulation& simulation,
	bool useTimeDisplacement = false, 
	double timeDisplacement = 0);

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
    const Simulation& m_simulation;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
