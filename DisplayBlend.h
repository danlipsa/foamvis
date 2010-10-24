/**
 * @file   DisplayBlend.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayBlend class
 */

#ifndef __DISPLAY_BLEND_H__
#define __DISPLAY_BLEND_H__

class GLWidget;

class DisplayBlend
{
public:
    DisplayBlend (GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }
    QGLFramebufferObject& GetCurrent ()
    {
	return *m_current;
    }

    void Init (const QSize& size);
    void End ();
    void Step (bool blend);
private:
    /**
     * FBO displayed on the screen.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_current;
    /**
     * Blended images of previous steps.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_previous;
    GLWidget& m_glWidget;
};

#endif //__DISPLAY_BLEND_H__

// Local Variables:
// mode: c++
// End:
