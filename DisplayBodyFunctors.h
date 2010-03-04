/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Functors to display a body
 */

#include "GLWidget.h"
#include "DisplayFace.h"

#ifndef __DISPLAY_BODY_FUNCTORS_H__
#define __DISPLAY_BODY_FUNCTORS_H__


/**
 * Functor used to display a body
 */
class displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    displayBody (GLWidget& widget) : m_widget (widget)
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
    /**
     * Returns the widget where we display
     */
    GLWidget& GetWidget () {return m_widget;}
    
protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (Body* b) = 0;
private:
    /**
     * Where to display the body
     */
    GLWidget& m_widget;
};

/**
 * Functor that displays the center of a bubble
 */
class displayBodyCenter : public displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the center of the bubble
     */
    displayBodyCenter (GLWidget& widget) : displayBody (widget) {}
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
class displayBodyWithFace : public displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    displayBodyWithFace (GLWidget& widget, const DisplayFace& df) : 
	displayBody (widget), m_df(df)
    {}
protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (Body* b)
    {
	vector<OrientedFace*> v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (), m_df);
    }
private:
    const DisplayFace& m_df;
};


/**
 * Functor that displays a body center given the index of the body
 */
class displayBodyCenterFromData : public displayBodyCenter
{
public:
    /**
     * Constructor
     * @param widget where to display the body center
     * @param index what body to display
     */
    displayBodyCenterFromData (GLWidget& widget, unsigned int index) :
    displayBodyCenter (widget), m_index (index) {}
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
