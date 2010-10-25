/**
 * @file   DisplayAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayAverage class
 */

#include "DisplayAverage.h"

void DisplayAverage::Init (const QSize& size)
{
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
}

void DisplayAverage::Release ()
{
    m_new.reset ();
    m_old.reset ();
}
