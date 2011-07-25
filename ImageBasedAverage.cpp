/**
 * @file   ImageBasedAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the ImageBasedAverage class 
 *
 */

#include "AverageShaders.h"
#include "Debug.h"
#include "DebugStream.h"
#include "ImageBasedAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "ShaderProgram.h"
#include "ViewSettings.h"

// Private classes/functions
// ======================================================================

// ImageBasedAverage Methods
// ======================================================================

boost::shared_ptr<ShaderProgram> ImageBasedAverage::m_initShaderProgram;
boost::shared_ptr<StoreShaderProgram> ImageBasedAverage::m_storeShaderProgram;
boost::shared_ptr<AddShaderProgram> ImageBasedAverage::m_addShaderProgram;
boost::shared_ptr<AddShaderProgram> ImageBasedAverage::m_removeShaderProgram;

void ImageBasedAverage::init (ViewNumber::Enum viewNumber)
{
    Average::init (viewNumber);
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    QSize size (viewRect.width (), viewRect.height ());
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    m_step.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D, 
	    GL_RGBA32F));

    if (m_step->attachment () != QGLFramebufferObject::CombinedDepthStencil)
	cdbg << "No stencil attachement available" << endl;

    m_current.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_previous.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_debug.reset (new QGLFramebufferObject (size));
    glPopAttrib ();
    clear (viewRect);
}

void ImageBasedAverage::clear (const G3D::Rect2D& viewRect)
{
    m_step->bind ();ClearColorStencilBuffers (Qt::black, 0);
    m_step->release ();

    m_current->bind ();ClearColorBuffer (Qt::black);
    m_current->release ();
    clearColorBufferMinMax (viewRect, m_previous);
}

void ImageBasedAverage::Release ()
{
    m_step.reset ();
    m_current.reset ();
    m_previous.reset ();
    m_debug.reset ();
}


typedef void (ImageBasedAverage::*Operation) (const G3D::Rect2D& viewRect);

void ImageBasedAverage::addStep (ViewNumber::Enum viewNumber, size_t time)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, time);
    //save (viewRect, *m_step, "step", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    addStepToCurrent (viewRect);
    //save (viewRect, *m_current, "current", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewRect, *m_previous, "previous", time, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);    
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep");
}

void ImageBasedAverage::removeStep (ViewNumber::Enum viewNumber, size_t time)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, time);
    //save (viewRect, *m_step, "step_", timey - m_timeWindow,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    removeStepFromCurrent (viewRect);
    //save (viewRect, *m_current, "current_", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewRect, *m_previous, "previous_", time, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep");
}


void ImageBasedAverage::renderToStep (ViewNumber::Enum viewNumber, size_t time)
{
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect ();
    glPushMatrix ();
    GetGLWidget ().ModelViewTransform (viewNumber, time);
    glViewport (0, 0, viewRect.width (), viewRect.height ());
    clearColorBufferMinMax (viewRect, m_step);
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    m_storeShaderProgram->Bind ();
    const Foam& foam = GetGLWidget ().GetFoamAlongTime ().GetFoam (time);
    const Foam::Bodies& bodies = foam.GetBodies ();
    writeFacesValues (viewNumber, bodies);
    m_storeShaderProgram->release ();
    m_step->release ();
    glPopMatrix ();
}

void ImageBasedAverage::addStepToCurrent (const G3D::Rect2D& viewRect)
{
    m_current->bind ();
    m_addShaderProgram->Bind ();

    // bind previous texture
    glActiveTexture (TextureEnum (m_addShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_addShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	*m_step);
    m_addShaderProgram->release ();
    m_current->release ();
}


void ImageBasedAverage::removeStepFromCurrent (const G3D::Rect2D& viewRect)
{
    m_current->bind ();
    m_removeShaderProgram->Bind ();

    // bind previous texture
    glActiveTexture (TextureEnum (m_removeShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_removeShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()),
	*m_step);
    m_removeShaderProgram->release ();
    m_current->release ();
}


void ImageBasedAverage::copyCurrentToPrevious ()
{
    QSize size = m_current->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_previous.get (), rect, m_current.get (), rect);
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
void ImageBasedAverage::clearColorBufferMinMax (
    const G3D::Rect2D& viewRect,
    const boost::scoped_ptr<QGLFramebufferObject>& fbo)
{
    fbo->bind ();
    m_initShaderProgram->Bind ();
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (0, 0, viewRect.width (), viewRect.height ());
    glPushMatrix ();
    {
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	{
	    glLoadIdentity ();
	    glBegin (GL_QUADS);
	    glVertex3i (-1, -1, -1);
	    glVertex3i (1, -1, -1);
	    glVertex3i (1, 1, -1);
	    glVertex3i (-1, 1, -1);
	    glEnd ();
	}
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
    }
    glPopAttrib ();
    glPopMatrix ();
    m_initShaderProgram->release ();
    fbo->release ();
}

void ImageBasedAverage::Display (ViewNumber::Enum viewNumber, 
			     StatisticsType::Enum displayType)
{
    if (! m_current)
	return;
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    display (viewRect, minMax.first, minMax.second, displayType, *m_current);
}

void ImageBasedAverage::DisplayAndRotate (ViewNumber::Enum viewNumber,
				      StatisticsType::Enum displayType, 
				      G3D::Vector2 rotationCenter, 
				      float angleDegrees)
{
    if (! m_current.get ())
	return;
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    displayAndRotate (
	viewRect, minMax.first, minMax.second, displayType, *m_current,
	rotationCenter, angleDegrees);
}


void ImageBasedAverage::save (
    const G3D::Rect2D& viewRect,
    QGLFramebufferObject& fbo, const char* postfix, size_t timeStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    // render to the debug buffer
    m_debug->bind ();
    display (G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	     minValue, maxValue, displayType, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << "images/" 
	 << setfill ('0') << setw (4) << timeStep << postfix << ".png";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}


void ImageBasedAverage::writeFacesValues (ViewNumber::Enum viewNumber, 
				      const Foam::Bodies& bodies)
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT |
		  GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (
	GL_TEXTURE_1D, 
	GetGLWidget ().GetViewSettings (viewNumber).GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceBodyPropertyColor<
	      SetterValueVertexAttribute>, SetterValueVertexAttribute> (
		  GetGLWidget (), 
		  GetGLWidget ().GetViewSettings (viewNumber).
		  GetBodySelector (), 
		  SetterValueVertexAttribute (
		      GetGLWidget (), viewNumber, m_storeShaderProgram.get (),
		      m_storeShaderProgram->GetVValueIndex ()),
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}

void ImageBasedAverage::glActiveTexture (GLenum texture) const
{
    const_cast<GLWidget&>(GetGLWidget ()).glActiveTexture (texture);
}

pair<double, double> ImageBasedAverage::getStatisticsMinMax (
    ViewNumber::Enum view) const
{
    double minValue, maxValue;
    if (GetGLWidget ().GetViewSettings (view).GetStatisticsType () == 
	StatisticsType::COUNT)
    {
	minValue = 0;
	maxValue = GetGLWidget ().GetFoamAlongTime ().GetTimeSteps ();
    }
    else
    {
	minValue = GetGLWidget ().GetFoamAlongTime ().GetMin (
	    GetGLWidget ().GetViewSettings (view).GetBodyProperty ());
	maxValue = GetGLWidget ().GetFoamAlongTime ().GetMax (
	    GetGLWidget ().GetViewSettings (view).GetBodyProperty ());
    }
    return pair<double, double> (minValue, maxValue);
}
