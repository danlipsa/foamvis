/**
 * @file   DisplayBlend.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayBlend class
 */

#ifndef __DISPLAY_BLEND_H__
#define __DISPLAY_BLEND_H__

#include "DisplayElement.h"

class GLWidget;

class DisplayBlend : public DisplayElement
{
public:
    DisplayBlend (const GLWidget& glWidget) :
	DisplayElement (glWidget)
    {
    }

    void Init (const QSize& size);
    void Release ();
    void Step (bool blendPrevious);
    void Display () const;

private:
    /**
     * FBO displayed on the screen.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_current;
    /**
     * Blended images of previous steps.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_previous;
};

#endif //__DISPLAY_BLEND_H__

// Local Variables:
// mode: c++
// End:
