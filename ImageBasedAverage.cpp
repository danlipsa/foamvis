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

template<typename PropertySetter> boost::shared_ptr<ShaderProgram> 
ImageBasedAverage<PropertySetter>::m_initShaderProgram;

template<typename PropertySetter> boost::shared_ptr<StoreShaderProgram> 
ImageBasedAverage<PropertySetter>::m_storeShaderProgram;

template<typename PropertySetter> boost::shared_ptr<AddShaderProgram> 
ImageBasedAverage<PropertySetter>::m_addShaderProgram;

template<typename PropertySetter> boost::shared_ptr<AddShaderProgram> 
ImageBasedAverage<PropertySetter>::m_removeShaderProgram;

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::init (ViewNumber::Enum viewNumber)
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

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::clear (const G3D::Rect2D& viewRect)
{
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    m_step->release ();

    m_current->bind ();
    ClearColorBuffer (Qt::black);
    m_current->release ();
    
    clearColorBufferMinMax (viewRect, m_previous);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::Release ()
{
    m_step.reset ();
    m_current.reset ();
    m_previous.reset ();
    m_debug.reset ();
}


template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::addStep (
    ViewNumber::Enum viewNumber, size_t time)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, time);
    //save (viewNumber, *m_step, "step", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    addStepToCurrent (viewNumber);
    //save (viewNumber, *m_current, "current", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewNumber, *m_previous, "previous", time, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);    
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::removeStep (
    ViewNumber::Enum viewNumber, size_t time)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, time);
    //save (viewNumber, *m_step, "step_", timey - m_timeWindow,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    removeStepFromCurrent (viewNumber);
    //save (viewNumber, *m_current, "current_", time,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewNumber, *m_previous, "previous_", time, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::renderToStep (
    ViewNumber::Enum viewNumber, size_t time)
{
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect ();
    //glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    GetGLWidget ().ModelViewTransform (viewNumber, time);
    glViewport (0, 0, viewRect.width (), viewRect.height ());
    clearColorBufferMinMax (viewRect, m_step);
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    const Foam& foam = GetGLWidget ().GetFoamAlongTime ().GetFoam (time);
    const Foam::Bodies& bodies = foam.GetBodies ();
    m_storeShaderProgram->Bind ();
    writeFacesValues (viewNumber, bodies);
    m_storeShaderProgram->release ();
    m_step->release ();
    glPopMatrix ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::addStepToCurrent (
    ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    m_current->bind ();
    m_addShaderProgram->Bind ();

    // activate texture unit 1
    glActiveTexture (TextureEnum (m_addShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // activate texture unit 2
    glActiveTexture (TextureEnum (m_addShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);

    ActivateShader (
	viewNumber,
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()));
    m_addShaderProgram->release ();
    m_current->release ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::removeStepFromCurrent (
    ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    m_current->bind ();
    m_removeShaderProgram->Bind ();

    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_removeShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // activate texture unit 2
    glActiveTexture (TextureEnum (m_removeShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());

    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    ActivateShader (
	viewNumber, 
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()));
    m_removeShaderProgram->release ();
    m_current->release ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::copyCurrentToPrevious ()
{
    QSize size = m_current->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_previous.get (), rect, m_current.get (), rect);
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::clearColorBufferMinMax (
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

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::RotateAndDisplay (
    ViewNumber::Enum viewNumber,
    StatisticsType::Enum displayType, 
    G3D::Vector2 rotationCenter, 
    float angleDegrees)
{
    if (! m_current)
	return;
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    rotateAndDisplay (
	viewNumber, viewRect, minMax.first, minMax.second, displayType, 
	*m_current, rotationCenter, angleDegrees);
}

// @todo: save does not work anymore 20110731
template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::save (
    ViewNumber::Enum viewNumber,
    QGLFramebufferObject& fbo, const char* postfix, size_t timeStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    // render to the debug buffer
    m_debug->bind ();
    rotateAndDisplay (
	viewNumber,
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	minValue, maxValue, displayType, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << "images/" 
	 << setfill ('0') << setw (4) << timeStep << postfix << ".png";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::writeFacesValues (
    ViewNumber::Enum viewNumber, const Foam::Bodies& bodies)
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
    for_each (
	bodies.begin (), bodies.end (),
	DisplayBody<DisplayFaceBodyPropertyColor<PropertySetter>,
	PropertySetter> (
	    GetGLWidget (), 
	    GetGLWidget ().GetViewSettings (viewNumber).GetBodySelector (), 
	    PropertySetter (
		GetGLWidget (), viewNumber, m_storeShaderProgram.get (),
		m_storeShaderProgram->GetVValueLocation ()),
	    DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::glActiveTexture (GLenum texture) const
{
    const_cast<GLWidget&>(GetGLWidget ()).glActiveTexture (texture);
}

template<typename PropertySetter>
pair<double, double> ImageBasedAverage<PropertySetter>::getStatisticsMinMax (
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


// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::ActivateShader (
    ViewNumber::Enum viewNumber,
    G3D::Rect2D destRect, 
    G3D::Vector2 rotationCenter, float angleDegrees)
{
    G3D::AABox srcAABox = GetGLWidget ().CalculateViewingVolume (viewNumber);
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (destRect.x0 (), destRect.y0 (),
		destRect.width (), destRect.height ());
    //glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    GetGLWidget ().EyeTransform (viewNumber);
    if (angleDegrees != 0)
    {
	glTranslate (rotationCenter);
	glRotatef (angleDegrees, 0, 0, 1);	
	glTranslate (-rotationCenter);
    }    
    G3D::Rect2D srcRect = G3D::Rect2D::xyxy (srcAABox.low ().xy (), 
					     srcAABox.high ().xy ());
    float z = (srcAABox.low ().z + srcAABox.high ().z) / 2;
    glBegin (GL_QUADS);
    glTexCoord2i (0, 0);
    ::glVertex (G3D::Vector3 (srcRect.x0y0 (), z));
    glTexCoord2i (1, 0);
    ::glVertex (G3D::Vector3 (srcRect.x1y0 (), z));
    glTexCoord2i (1, 1);
    ::glVertex (G3D::Vector3 (srcRect.x1y1 (), z));
    glTexCoord2i (0, 1);
    ::glVertex (G3D::Vector3 (srcRect.x0y1 (), z));
    glEnd ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}


// Template instantiations
//======================================================================
/// @cond
template class ImageBasedAverage<SetterVertexAttribute>;
template class ImageBasedAverage<SetterDeformationTensor>;
/// @endcond
