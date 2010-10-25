/**
 * @file   DisplayAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayAverage class
 */

#ifndef __DISPLAY_AVERAGE_H__
#define __DISPLAY_AVERAGE_H__

#include "DisplayElement.h"

class Body;
class Foam;
class GLWidget;


class DisplayAverage : public DisplayElement
{
public:
    DisplayAverage (const GLWidget& glWidget) :
	DisplayElement (glWidget)
    {
    }

    void Init (const QSize& size);
    void Release ();
    void Step (const Foam& foam);
    void Display ();

private:
    template<typename displaySameEdges>
    void displayFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies) const;


private:
    /**
     * Stores the sum and count of values
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * Stores the sum and count of the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
};

#endif //__DISPLAY_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
