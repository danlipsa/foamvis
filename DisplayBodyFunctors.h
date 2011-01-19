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
#include "StripIterator.h"
class DisplayEdge;
class Body;
class BodyAlongTime;
class BodySelector;
class StripIterator;

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
    
    inline void operator () (const map <size_t, 
			     boost::shared_ptr<BodyAlongTime> >::value_type& p)
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

    void copySegments (StripIterator& it);

    void speedStep (
	const StripIterator::Point& beforeBegin,
	const StripIterator::Point& begin,
	const StripIterator::Point& end,
	const StripIterator::Point& afterEnd);

    void valueStep (
	const StripIterator::Point& beforeBegin,
	const StripIterator::Point& begin,
	const StripIterator::Point& end,
	const StripIterator::Point& afterEnd);

    void halfValueStep (const StripIterator::Point& p, G3D::Vector3 middle,
			bool swapPoints);

    void displaySegments ();

    G3D::Vector3 getPoint (StripIterator::Point p, bool useTimeDisplacement,
			   double timeDisplacement);

    QColor focusContextColor (bool focus, const QColor& color);

    void storeFocusSegment (
	double value, G3D::Vector3 begin, G3D::Vector3 end);

    void storeContextSegment (const QColor& color, bool focus,
			      G3D::Vector3 begin, G3D::Vector3 end);

    void displayContextSegment (const ContextSegment& coloredSegment);

    void displayFocusSegment (const FocusSegment& texturedSegment);

private:
    DisplaySegment m_displaySegment;
    vector<FocusSegment> m_focusSegments;
    vector<ContextSegment> m_contextSegments;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
