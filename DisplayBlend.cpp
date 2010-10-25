/**
 * @file   DisplayBlend.cpp
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayBlend class
 */

#include "DisplayBlend.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"


void DisplayBlend::Init (const QSize& size)
{
    m_current.reset (new QGLFramebufferObject (size));
    m_previous.reset (new QGLFramebufferObject (size));
    Step (false);
}

void DisplayBlend::Release ()
{
    m_current.reset ();
    m_previous.reset ();
}

void DisplayBlend::Step (bool blend)
{
    QSize size = m_current->size ();
    {
	glPushMatrix ();
	glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT);
	m_glWidget.ViewportTransform (size.width (), size.height ());
	m_glWidget.ModelViewTransformNoRotation ();
	{
	    m_current->bind ();
	    // render to the current buffer
	    glClear(GL_COLOR_BUFFER_BIT);
	    m_glWidget.Display ();

	    if (blend)
	    {
		// blend from the previous buffer	    
		glEnable (GL_BLEND);	
		glBlendFunc (GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA);
		const_cast<GLWidget&>(m_glWidget).glBlendColor (
		    0, 0, 0, m_glWidget.GetSrcAlphaBlend ());
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		m_glWidget.RenderFromFbo (*m_previous);
		glDisable (GL_BLEND);
	    }
	    m_current->release ();
	}
	//m_current->toImage ().save ("current1.jpg");
        // copy current --> previous buffer
	QRect rect (QPoint (0, 0), size);
	QGLFramebufferObject::blitFramebuffer (
	    m_previous.get (), rect, m_current.get (), rect);
	//m_previous->toImage ().save ("previous1.jpg");
	glPopAttrib ();
	glPopMatrix ();
    }
    detectOpenGLError ();
}

void DisplayBlend::Display () const
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    m_glWidget.RenderFromFbo (*m_current);    
}
