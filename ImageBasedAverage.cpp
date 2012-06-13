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
#include "Simulation.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "ShaderProgram.h"
#include "ScalarAverage.h"
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
void ImageBasedAverage<PropertySetter>::AverageInit (
    ViewNumber::Enum viewNumber)
{
    Average::AverageInit (viewNumber);
    const G3D::Rect2D extendedArea = EncloseRotation (
	GetWidgetGl ().GetViewRect (viewNumber));
    QSize size (extendedArea.width (), extendedArea.height ());
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    m_fbos.m_step.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D, 
	    GL_RGBA32F));

    RuntimeAssert (m_fbos.m_step->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    RuntimeAssert (m_fbos.m_step->attachment () == 
		   QGLFramebufferObject::CombinedDepthStencil,
		   "No stencil attachement available:" + m_id);

    m_fbos.m_current.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    RuntimeAssert (m_fbos.m_current->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    m_fbos.m_previous.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    RuntimeAssert (m_fbos.m_previous->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    m_fbos.m_debug.reset (new QGLFramebufferObject (size));
    RuntimeAssert (m_fbos.m_debug->isValid (), 
		   "Framebuffer initialization failed:" + m_id);
    glPopAttrib ();
    clear (viewNumber);
    WarnOnOpenGLError ("ImageBasedAverage::init");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::clear (ViewNumber::Enum viewNumber)
{
    //const size_t FAKE_TIMESTEP = -1;
    pair<float,float> minMax = 
	GetWidgetGl ().GetRange (viewNumber);
    m_fbos.m_step->bind ();
    ClearColorStencilBuffers (getStepClearColor (), 0);
    m_fbos.m_step->release ();
    //save (
    //viewNumber, make_pair (m_fbos.m_step, m_scalarAverageFbos.m_step), 
    //"step", FAKE_TIMESTEP, 0, 
    //minMax.first, minMax.second, ComputationType::AVERAGE);

    initFramebuffer (viewNumber, m_fbos.m_current);
    //save (viewNumber, 
    //make_pair (m_fbos.m_current, m_scalarAverageFbos.m_current), 
    //"current", FAKE_TIMESTEP, 0,
    //minMax.first, minMax.second, ComputationType::AVERAGE);
    
    initFramebuffer (viewNumber, m_fbos.m_previous);
    // save (viewNumber, 
    // 	  make_pair (m_fbos.m_previous, m_scalarAverageFbos.m_previous), 
    // 	  "previous", FAKE_TIMESTEP + 1,
    // 	  minMax.first, minMax.second, ComputationType::AVERAGE);
    WarnOnOpenGLError ("ImageBasedAverage::clear");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::AverageRelease ()
{
    m_fbos.m_step.reset ();
    m_fbos.m_current.reset ();
    m_fbos.m_previous.reset ();
    m_fbos.m_debug.reset ();
}


template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::addStep (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep)
{
    pair<float,float> minMax = 
	GetWidgetGl ().GetRange (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, timeStep, subStep);
    //save (
    //viewNumber, make_pair (m_fbos.m_step, m_scalarAverageFbos.m_step), 
    //"step", timeStep, subStep, 
    //minMax.first, minMax.second, ComputationType::AVERAGE);

    currentIsPreviousPlusStep (viewNumber);
    //save (viewNumber, 
    //make_pair (m_fbos.m_current, m_scalarAverageFbos.m_current), 
    //"current", timeStep, subStep,
    //minMax.first, minMax.second, ComputationType::AVERAGE);
    //cdbg << "addStep: " << timeStep << "-" << subStep << endl;

    copyCurrentToPrevious ();
    // save (viewNumber, 
    // 	  make_pair (m_fbos.m_previous, m_scalarAverageFbos.m_previous), 
    // 	  "previous", timeStep + 1,
    // 	  minMax.first, minMax.second, ComputationType::AVERAGE);
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::removeStep (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep)
{
    pair<float,float> minMax = 
	GetWidgetGl ().GetRange (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, timeStep, subStep);
    //save (viewNumber, 
    //TensorScalarFbo (*m_step, *m_scalarAverage.m_step), 
    // "step", timeStep, minMax.first, minMax.second, ComputationType::AVERAGE);

    currentIsPreviousMinusStep (viewNumber);
    //save (viewNumber, 
    //make_pair (m_fbos.m_current, m_scalarAverageFbos.m_current), 
    //"current", timeStep, subStep,
    //minMax.first, minMax.second, ComputationType::AVERAGE);
    //cdbg << "removeStep: " << timeStep << "-" << subStep << endl;

    copyCurrentToPrevious ();
    //save (viewNumber, 
    //TensorScalarFbo (*m_previous, *m_scalarAverage.m_previous), 
    //"previous", timeStep + 1,
    //minMax.first, minMax.second, ComputationType::AVERAGE);

    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::removeStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::renderToStep (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep)
{
    const WidgetGl& widgetGl = GetWidgetGl ();
    G3D::Rect2D destRect = 
	EncloseRotation (widgetGl.GetViewRect (viewNumber));
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    widgetGl.ModelViewTransform (viewNumber, timeStep);
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    widgetGl.ProjectionTransform (
	viewNumber, ViewingVolumeOperation::ENCLOSE2D);
    glViewport (0, 0, destRect.width (), destRect.height ());    

    m_fbos.m_step->bind ();
    ClearColorStencilBuffers (getStepClearColor (), 0);
    writeStepValues (viewNumber, timeStep, subStep);
    m_fbos.m_step->release ();

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    WarnOnOpenGLError ("ImageBasedAverage::renderToStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::currentIsPreviousPlusStep (
    ViewNumber::Enum viewNumber)
{
    m_fbos.m_current->bind ();
    m_addShaderProgram->Bind ();

    // activate texture unit 1
    glActiveTexture (TextureEnum (m_addShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_fbos.m_previous->texture ());

    // activate texture unit 2
    glActiveTexture (TextureEnum (m_addShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_fbos.m_step->texture ());
    
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);

    GetWidgetGl ().ActivateViewShader (viewNumber);
    m_addShaderProgram->release ();
    m_fbos.m_current->release ();
    WarnOnOpenGLError ("ImageBasedAverage::currentIsPreviousPlusStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::currentIsPreviousMinusStep (
    ViewNumber::Enum viewNumber)
{
    m_fbos.m_current->bind ();
    m_removeShaderProgram->Bind ();

    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_removeShaderProgram->GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_fbos.m_previous->texture ());

    // activate texture unit 2
    glActiveTexture (TextureEnum (m_removeShaderProgram->GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_fbos.m_step->texture ());

    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    GetWidgetGl ().ActivateViewShader (viewNumber);
    m_removeShaderProgram->release ();
    m_fbos.m_current->release ();
    WarnOnOpenGLError ("ImageBasedAverage::currentIsPreviousMinusStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::copyCurrentToPrevious ()
{
    QSize size = m_fbos.m_current->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_fbos.m_previous.get (), rect, m_fbos.m_current.get (), rect);
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::initFramebuffer (
    ViewNumber::Enum viewNumber,
    const boost::shared_ptr<QGLFramebufferObject>& fbo)
{
    fbo->bind ();
    m_initShaderProgram->Bind ();
    GetWidgetGl ().ActivateViewShader (viewNumber);
    m_initShaderProgram->release ();
    fbo->release ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::AverageRotateAndDisplay (
    ViewNumber::Enum viewNumber,
    ComputationType::Enum displayType, 
    G3D::Vector2 rotationCenter, 
    float angleDegrees) const
{    
    ViewSettings& vs = GetWidgetGl ().GetViewSettings (viewNumber);
    glBindTexture (GL_TEXTURE_1D, vs.GetColorBarTexture ());
    pair<float,float> minMax = GetWidgetGl ().GetRange (viewNumber);
    rotateAndDisplay (
	viewNumber, minMax.first, minMax.second, displayType, 
	make_pair (m_fbos.m_current, m_scalarAverageFbos.m_current), 
	ViewingVolumeOperation::DONT_ENCLOSE2D, rotationCenter, angleDegrees);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::save (
    ViewNumber::Enum viewNumber,
    TensorScalarFbo fbo, const char* postfix, size_t timeStep, size_t subStep,
    GLfloat minValue, GLfloat maxValue, ComputationType::Enum displayType)
{
    // render to the debug buffer
    m_fbos.m_debug->bind ();
    ClearColorBuffer (Qt::white);
    rotateAndDisplay (
	viewNumber,	
	minValue, maxValue, displayType, fbo, ViewingVolumeOperation::ENCLOSE2D);
    m_fbos.m_debug->release ();
    ostringstream ostr;
    ostr << "images/" 
	 << m_id << setfill ('0') << setw (4) << timeStep << "-" 
	 << setw (2) << subStep << postfix << ".png";
    m_fbos.m_debug->toImage ().save (ostr.str ().c_str ());
    WarnOnOpenGLError ("ImageBasedAverage::save");
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::writeStepValues (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep)
{
    (void)subStep;
    ViewSettings& vs = GetWidgetGl ().GetViewSettings (viewNumber);
    const Foam& foam = 
	GetWidgetGl ().GetSimulation (viewNumber).GetFoam (timeStep);
    const Foam::Bodies& bodies = foam.GetBodies ();
    m_storeShaderProgram->Bind ();
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT );
    glEnable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);

    for_each (
	bodies.begin (), bodies.end (),
	DisplayBody<DisplayFaceBodyPropertyColor<PropertySetter>,
	PropertySetter> (
	    GetWidgetGl (), foam,
	    vs.GetBodySelector (), 
	    PropertySetter (
		GetWidgetGl (), viewNumber, m_storeShaderProgram.get (),
		m_storeShaderProgram->GetVValueLocation ()),
	    DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
    m_storeShaderProgram->release ();
    WarnOnOpenGLError ("ImageBasedAverage::writeStepValues:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::glActiveTexture (GLenum texture) const
{
    const_cast<WidgetGl&>(GetWidgetGl ()).glActiveTexture (texture);
}

// Template instantiations
//======================================================================
/// @cond
template class ImageBasedAverage<SetterVertexAttribute>;
template class ImageBasedAverage<SetterDeformation>;
template class ImageBasedAverage<SetterVelocity>;
template class ImageBasedAverage<SetterNop>;
/// @endcond
