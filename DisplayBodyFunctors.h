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
        unsigned int displayedBody = m_widget.GetDisplayedBody ();
        if (displayedBody == m_widget.DISPLAY_ALL ||
             b->GetOriginalIndex () == displayedBody)
        {
	    display (b);
        }
    }

protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (Body* b) = 0;
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
    virtual void display (Body* b)
    {
	G3D::Vector3 v = b->GetCenter ();
	glVertex3f(v.x, v.y, v.z);
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
    virtual void display (Body* b)
    {
	vector<OrientedFace*> v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (), displayFace(m_widget));
    }
};


/**
 * Functor that displays a body center given the index of the body
 */
class DisplayBodyCenterFromData : public DisplayBodyCenter
{
public:
    /**
     * Constructor
     * @param widget where to display the body center
     * @param index what body to display
     */
    DisplayBodyCenterFromData (GLWidget& widget, unsigned int index) :
    DisplayBodyCenter (widget), m_index (index) {}
    /**
     * Functor that displays a body center
     * @param data Data object that constains the body
     */
    void operator () (Data* data)
    {
	Body* body = data->GetBody (m_index);
	if (body != 0)
	    display (body);
    }
private:
    /**
     * What body to display
     */
    int m_index;
};

#endif //__DISPLAY_BODY_FUNCTORS_H__
