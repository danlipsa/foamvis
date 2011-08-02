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

    RuntimeAssert (m_step->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    RuntimeAssert (
	m_step->attachment () == QGLFramebufferObject::CombinedDepthStencil,
	"No stencil attachement available:" + m_id);

    m_current.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    RuntimeAssert (m_current->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    m_previous.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    RuntimeAssert (m_previous->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    m_debug.reset (new QGLFramebufferObject (size));
    RuntimeAssert (m_debug->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    glPopAttrib ();
    clear (viewNumber);
    WarnOnOpenGLError ("ImageBasedAverage::init");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::clear (ViewNumber::Enum viewNumber)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    m_step->release ();
    save (viewNumber, *m_step, "step", 0,
	  minMax.first, minMax.second, StatisticsType::AVERAGE);

    m_current->bind ();
    ClearColorBuffer (Qt::black);
    m_current->release ();
    save (viewNumber, *m_current, "current", 0,
	  minMax.first, minMax.second, StatisticsType::AVERAGE);
    
    initFramebuffer (viewNumber, m_previous);
    save (viewNumber, *m_previous, "previous", 0,
	  minMax.first, minMax.second, StatisticsType::AVERAGE);
    WarnOnOpenGLError ("ImageBasedAverage::clear");
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
    WarnOnOpenGLError ("ImageBasedAverage::addStep:" + m_id);
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
    WarnOnOpenGLError ("ImageBasedAverage::removeStep:" + m_id);
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
    initFramebuffer (viewNumber, m_step);
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    const Foam& foam = GetGLWidget ().GetFoamAlongTime ().GetFoam (time);
    const Foam::Bodies& bodies = foam.GetBodies ();
    writeFacesValues (viewNumber, bodies);
    m_step->release ();
    glPopMatrix ();
    WarnOnOpenGLError ("ImageBasedAverage::renderToStep:" + m_id);
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

    GetGLWidget ().ActivateShader (
	viewNumber,
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()));
    m_addShaderProgram->release ();
    m_current->release ();
    WarnOnOpenGLError ("ImageBasedAverage::addStepToCurrent:" + m_id);
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
    GetGLWidget ().ActivateShader (
	viewNumber, 
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()));
    m_removeShaderProgram->release ();
    m_current->release ();
    WarnOnOpenGLError ("ImageBasedAverage::removeStepFromCurrent:" + m_id);
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
void ImageBasedAverage<PropertySetter>::initFramebuffer (
    ViewNumber::Enum viewNumber,
    const boost::scoped_ptr<QGLFramebufferObject>& fbo)
{
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    fbo->bind ();
    m_initShaderProgram->Bind ();
    GetGLWidget ().ActivateShader (viewNumber, viewRect - viewRect.x0y0 ());
    m_initShaderProgram->release ();
    fbo->release ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::RotateAndDisplay (
    ViewNumber::Enum viewNumber,
    StatisticsType::Enum displayType, 
    G3D::Vector2 rotationCenter, 
    float angleDegrees) const
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
	 << m_id << setfill ('0') << setw (4) << timeStep << postfix << ".png";
    m_debug->toImage ().save (ostr.str ().c_str ());
    WarnOnOpenGLError ("ImageBasedAverage::save");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::writeFacesValues (
    ViewNumber::Enum viewNumber, const Foam::Bodies& bodies)
{
    m_storeShaderProgram->Bind ();
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT );
    glPolygonMode (GL_FRONT, GL_FILL);    
    glEnable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);

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
    m_storeShaderProgram->release ();
    WarnOnOpenGLError ("ImageBasedAverage::writeFacesValues:" + m_id);
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


// Template instantiations
//======================================================================
/// @cond
template class ImageBasedAverage<SetterVertexAttribute>;
template class ImageBasedAverage<SetterDeformationTensor>;
/// @endcond
