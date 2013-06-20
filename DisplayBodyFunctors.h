/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 * @brief Functors that display a bubble (body)
 * @ingroup display
 *
 * @defgroup display Display
 * Visualizes data
 */

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__

#include "DisplayElement.h"
#include "Enums.h"

class DisplaySegmentLine;
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
 * @brief Common functionality for displaying a bubble (focus/context)
 */
template <typename PropertySetter = SetterTextureCoordinate>
class DisplayBodyBase : public DisplayElementProperty<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayBodyBase (const Settings& settings, 
		     const BodySelector& bodySelector, 
		     PropertySetter propertySetter, Context::Enum context,
		     bool useZPos = false, double zPos = 0);

    static void BeginContext ();
    static void EndContext ();
    bool IsFocus (boost::shared_ptr<Body> b) const;
    DisplayElement::FocusContext GetFocusContext (
        boost::shared_ptr<Body> b) const
    {
	return  IsFocus (b) ? DisplayElement::FOCUS : DisplayElement::CONTEXT;
    }
    void operator () (boost::shared_ptr<Body> b);

protected:
    virtual void display (boost::shared_ptr<Body> b) = 0;

private:
    const BodySelector& m_bodySelector;
    Context::Enum m_context;
};


/**
 * @brief Displays 2D bubble deformation as an ellipse
 */
class DisplayBodyDeformation : public DisplayBodyBase<>
{
public:
    DisplayBodyDeformation (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	const BodySelector& bodySelector, float deformationSizeInitialRatio,
	bool useZPos = false, double zPos = 0);

protected:
    virtual void display (boost::shared_ptr<Body> b);

private:
    float m_deformationSizeInitialRatio;
};


/**
 * @brief Displays bubble velocity using a glyph
 */
class DisplayBodyVelocity : public DisplayBodyBase<>
{
public:
    DisplayBodyVelocity (
	const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
	const BodySelector& bodySelector, float bubbleSize, 
	float velocitySizeInitialRatio, float onePixelInObjectSpace, 
        GLUquadricObj* quadric, bool sameSize, bool clampingShown,
	bool useZPos = false, double zPos = 0);

protected:
    virtual void display (boost::shared_ptr<Body> b);

private:
    float m_bubbleDiameter;
    float m_onePixelInObjectSpace;
    float m_velocitySizeInitialRatio;
    bool m_sameSize;
    bool m_clampingShown;
    bool m_is2D;
    GLUquadricObj* m_quadric;
};

/**
 * @brief Displays the center of a bubble
 */
class DisplayBodyCenter : public DisplayBodyBase<>
{
public:
    /**
     * Constructor
     */
    DisplayBodyCenter (
	const Settings& settings,
	const BodySelector& bodySelector,
	bool useZPos = false, double zPos = 0);

protected:
    virtual void display (boost::shared_ptr<Body> b);
};


/**
 * @brief Displays a body by displaying all its faces. The function
 *        that displays a face is a parameter.
 */
template<typename displayFace, typename PropertySetter = SetterTextureCoordinate>
class DisplayBody : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayBody (
	const Settings& settings, bool is2D, const BodySelector& bodySelector,
        Context::Enum context = Context::UNSELECTED,
	ContextInvisible::Enum
	contextDisplay = ContextInvisible::USER_DEFINED,
	ViewNumber::Enum viewNumber = ViewNumber::VIEW0,
	bool useZPos = false, double zPos = 0);

    DisplayBody (
	const Settings& settings, const BodySelector& bodySelector,
	PropertySetter setter,
        Context::Enum context = Context::UNSELECTED,
	ContextInvisible::Enum contextDisplay = ContextInvisible::USER_DEFINED,
	bool useZPos = false, double zPos = 0);

protected:
    virtual void display (boost::shared_ptr<Body> b);
			
private:
    ContextInvisible::Enum m_contextInvisible;
};


/**
 * @brief Displays the bubble path for a certain body id
 *
 * @todo Use Simulation::GetBodyScalarValue instead of
 *       StripIterator functions
 */
template<typename PropertySetter = SetterTextureCoordinate,
	 typename DisplayEdge = DisplaySegmentLine>
class DisplayBubblePaths : public DisplayBodyBase<PropertySetter>
{
public:
    /**
     * Constructor
     */
    DisplayBubblePaths (
	const Settings& settings, ViewNumber::Enum view, bool is2D,
	const BodySelector& bodySelector, GLUquadricObj* quadric, 
	const Simulation& simulation, size_t begin, size_t end,
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

protected:
    virtual void display (boost::shared_ptr<Body> b)
    {
        (void)b;
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
    size_t m_timeBegin;
    size_t m_timeEnd;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
