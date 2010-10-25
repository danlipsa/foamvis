/**
 * @file   DisplayAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayAverage class
 */

#include "DisplayAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "GLWidget.h"

void DisplayAverage::Init (const QSize& size)
{
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D));
}

void DisplayAverage::Release ()
{
    m_new.reset ();
    m_old.reset ();
}

void DisplayAverage::Step (const Foam& foam)
{
    QSize size = m_new->size ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    {
	glPushMatrix ();
	glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	m_glWidget.ViewportTransform (size.width (), size.height ());
	m_glWidget.ModelViewTransformNoRotation ();
	{
	    m_new->bind ();
	    glClearColor (Qt::yellow);
	    // render to the new buffer
	    glClear(GL_COLOR_BUFFER_BIT);
	    if (foam.IsQuadratic ())
		displayFacesValues<DisplaySameEdges> (bodies);
	    else
		displayFacesValues<DisplaySameTriangles> (bodies);
	    m_new->release ();
	}
	m_new->toImage ().save ("new.jpg");
        // copy current --> previous buffer
	QRect rect (QPoint (0, 0), size);
	QGLFramebufferObject::blitFramebuffer (
	    m_old.get (), rect, m_new.get (), rect);
	m_old->toImage ().save ("old.jpg");
	glPopAttrib ();
	glPopMatrix ();
    }
    detectOpenGLError ();
}

void DisplayAverage::Display ()
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    m_glWidget.RenderFromFbo (*m_new);
}

template<typename displaySameEdges>
void DisplayAverage::displayFacesValues (const Foam::Bodies& bodies) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, m_glWidget.GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceWithColor<displaySameEdges> > (
		  m_glWidget, AllBodiesSelected (), 
		  DisplayElement::INVISIBLE_CONTEXT, 
		  m_glWidget.GetFacesColor ()));
    glPopAttrib ();
}
