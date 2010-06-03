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
#include "Foam.h"

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
     * Functor used to display a body
     * @param b the body to be displayed
     */
    void operator () (Body* b)
    {
        if (m_widget.IsDisplayedBody (b))
	    display (b, FOCUS);
	else
	{
	    
	    glEnable (GL_BLEND);
	    glDepthMask (GL_FALSE);
	    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    
	    display (b, CONTEXT);

	    glDepthMask (GL_TRUE);
	    glDisable (GL_BLEND);
	}
    }

protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (Body* b, FocusContext fc) = 0;
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
    virtual void display (Body* b, FocusContext fc)
    {
	(void)fc;
	G3D::Vector3 v = b->GetCenter ();
	glVertex(v);
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
    DisplayBody (GLWidget& widget) : 
    DisplayBodyBase (widget)
    {}
protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (Body* b, FocusContext bodyFc)
    {
	vector<OrientedFace*> v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (), displayFace(m_widget, bodyFc));
    }
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
    DisplayCenterPath (GLWidget& widget) : m_widget (widget) {}
    /**
     * Displays the center path for a certain body
     * @param bodyId what body to display the center path for
     */
    void operator () (size_t bodyId)
    {	
	BodyAlongTime& bat = m_widget.GetBodyAlongTime (bodyId);
	const BodyAlongTime::Bodies& bodyAlongTime = bat.GetBodies ();
	BodyAlongTime::Bodies::const_iterator begin = bodyAlongTime.begin ();
	BOOST_FOREACH (size_t wrapIndex, bat.GetWraps ())
	{
	    BodyAlongTime::Bodies::const_iterator end = 
		bodyAlongTime.begin () + wrapIndex;
	    glBegin(GL_LINE_STRIP);
	    for_each (begin, end, DisplayBodyCenter (m_widget));
	    glEnd ();
	    begin = end + 1;
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
    /**
     * Where to display the center path
     */
    GLWidget& m_widget;
};



#endif //__DISPLAY_BODY_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
