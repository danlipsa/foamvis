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
#include "ImageBasedAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "Settings.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "ShaderProgram.h"
#include "ScalarAverage2D.h"
#include "ViewSettings.h"



// Private classes/functions
// ======================================================================

size_t getNumberOfComponents (GLenum format)
{
    switch (format)
    {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
        // read a scalar or the count
        return 1;
    case GL_RGB:
        // read a 2D vector (RG contain the vector components)
        return 3;
    case GL_RGBA:
        // read a 2D tensor
        return 4;
    default:
        ThrowException ("Invalid read format: ", format);
        return 0;
    }
}

const size_t FAKE_TIMESTEP = -1;

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
ImageBasedAverage<PropertySetter>::ImageBasedAverage (
    ViewNumber::Enum viewNumber,
    const WidgetGl& widgetGl, string id, QColor stepClearColor,
    FramebufferObjects& countFbos, size_t countIndex) :

    Average (viewNumber, 
	     widgetGl.GetSettingsPtr (), widgetGl.GetSimulationGroupPtr ()), 
    m_countFbos (countFbos), 
    m_countIndex (countIndex),
    m_id (id),
    m_stepClearColor (stepClearColor),
    m_widgetGl (widgetGl)
{
}


template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::AverageInit ()
{
    try
    {
	Average::AverageInit ();
	const G3D::Rect2D extendedArea = EncloseRotation (
	    GetWidgetGl ().GetViewRect (GetViewNumber ()));
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
	clear ();
	WarnOnOpenGLError ("ImageBasedAverage::init");
    }
    catch (exception& e)
    {
	glPopAttrib ();
	cdbg << e.what () << endl;
    }
}

template<typename PropertySetter>
G3D::Rect2D ImageBasedAverage<PropertySetter>::GetWindowCoord () const
{
    G3D::Rect2D windowCoord = 
        G3D::Rect2D::xywh(G3D::Vector2::zero (), 
                          ToG3D (this->m_fbos.m_current->size ()));
    return windowCoord;
}



template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::clear ()
{
    pair<float,float> minMax = 
	GetWidgetGl ().GetRange (GetViewNumber ());
    m_fbos.m_step->bind ();
    ClearColorStencilBuffers (getStepClearColor (), 0);
    m_fbos.m_step->release ();
    __LOG__ (save (FbosCountFbos (m_fbos.m_step, m_countFbos.m_step, 
                                  m_countIndex),
                   "step", FAKE_TIMESTEP, 0,
                   minMax.first, minMax.second, StatisticsType::AVERAGE););

    initFramebuffer (m_fbos.m_current);
    __LOG__ (save (FbosCountFbos (m_fbos.m_current, m_countFbos.m_current, 
                                  m_countIndex), 
                   "current", FAKE_TIMESTEP, 0,
                   minMax.first, minMax.second, StatisticsType::AVERAGE););
    
    initFramebuffer (m_fbos.m_previous);
    __LOG__ (save (FbosCountFbos (m_fbos.m_previous, m_countFbos.m_previous, 
                                  m_countIndex), 
                   "previous", FAKE_TIMESTEP + 1, 0,
                   minMax.first, minMax.second, StatisticsType::AVERAGE);)
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
void ImageBasedAverage<PropertySetter>::addStep (size_t timeStep, size_t subStep)
{
    __ENABLE_LOGGING__;
    pair<float,float> minMax = GetWidgetGl ().GetRange (GetViewNumber ());
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (timeStep, subStep);
    __LOG__ (save 
             (FbosCountFbos (m_fbos.m_step, m_countFbos.m_step, m_countIndex), 
              "step", timeStep, subStep, 
              minMax.first, minMax.second, StatisticsType::AVERAGE);)

    currentIsPreviousPlusStep ();
    __LOG__ (
        save (FbosCountFbos (m_fbos.m_current, m_countFbos.m_current, 
                             m_countIndex), 
              "current", timeStep, subStep,
              minMax.first, minMax.second, StatisticsType::AVERAGE););
        
    copyCurrentToPrevious ();
    __LOG__ (save (
                 FbosCountFbos (m_fbos.m_previous, m_countFbos.m_previous, 
                                m_countIndex), 
                 "previous", timeStep + 1, subStep,
                 minMax.first, minMax.second, StatisticsType::AVERAGE););
    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::addStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::removeStep (
    size_t timeStep, size_t subStep)
{
    pair<float,float> minMax = GetWidgetGl ().GetRange (GetViewNumber ());
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (timeStep, subStep);
    __LOG__(save (FbosCountFbos (m_fbos.m_step, m_countFbos.m_step, 
                                 m_countIndex), 
                  "step", timeStep, subStep, minMax.first, minMax.second, 
                  StatisticsType::AVERAGE););

    currentIsPreviousMinusStep ();
    __LOG__ (save (FbosCountFbos (m_fbos.m_current, m_countFbos.m_current, 
                                  m_countIndex), 
                   "current", timeStep, subStep,
                   minMax.first, minMax.second, StatisticsType::AVERAGE);
             cdbg << "removeStep: " << timeStep << "-" << subStep << endl;);

    copyCurrentToPrevious ();
    __LOG__ (save (FbosCountFbos (m_fbos.m_previous, m_countFbos.m_previous, 
                                  m_countIndex), 
                   "previous", timeStep + 1, subStep,
                   minMax.first, minMax.second, StatisticsType::AVERAGE););

    glPopAttrib ();
    WarnOnOpenGLError ("ImageBasedAverage::removeStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::renderToStep (
    size_t timeStep, size_t subStep)
{
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();

    GetWidgetGl ().AllTransformAverage (GetViewNumber (), timeStep, 
                                        WidgetGl::ROTATE_FOR_AXIS_ORDER);
    m_fbos.m_step->bind ();
    ClearColorStencilBuffers (getStepClearColor (), 0);
    writeStepValues (GetViewNumber (), timeStep, subStep);
    m_fbos.m_step->release ();

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    WarnOnOpenGLError ("ImageBasedAverage::renderToStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::currentIsPreviousPlusStep ()
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

    GetWidgetGl ().ActivateViewShader (GetViewNumber ());
    m_addShaderProgram->release ();
    m_fbos.m_current->release ();
    WarnOnOpenGLError ("ImageBasedAverage::currentIsPreviousPlusStep:" + m_id);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::currentIsPreviousMinusStep ()
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
    GetWidgetGl ().ActivateViewShader (GetViewNumber ());
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
    const boost::shared_ptr<QGLFramebufferObject>& fbo)
{
    RuntimeAssert (m_initShaderProgram != 0, "Null shader program");
    fbo->bind ();
    m_initShaderProgram->Bind ();
    GetWidgetGl ().ActivateViewShader (GetViewNumber ());
    m_initShaderProgram->release ();
    fbo->release ();
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::AverageRotateAndDisplay (
    StatisticsType::Enum displayType, 
    G3D::Vector2 rotationCenter, 
    float angleDegrees) const
{    
    glBindTexture (GL_TEXTURE_1D, 
		   GetWidgetGl ().GetColorMapScalarTexture (GetViewNumber ()));
    pair<float,float> minMax = GetWidgetGl ().GetRange (GetViewNumber ());
    rotateAndDisplay (
	minMax.first, minMax.second, displayType, 
	FbosCountFbos (m_fbos.m_current, m_countFbos.m_current, m_countIndex), 
	ViewingVolumeOperation::DONT_ENCLOSE2D, rotationCenter, angleDegrees);
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::writeStepValues (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep)
{
    (void)subStep;
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    const Foam& foam = GetFoam (timeStep);
    const Foam::Bodies& bodies = foam.GetBodies ();
    m_storeShaderProgram->Bind ();
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT );
    glEnable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);

    for_each (
	bodies.begin (), bodies.end (),
	DisplayBody<DisplayFaceBodyScalarColor<PropertySetter>,
	PropertySetter> (
	    GetSettings (), *vs.GetBodySelector (), 
	    PropertySetter (GetSettings (), 
			    viewNumber, GetSimulation ().Is2D (),
                            m_storeShaderProgram.get (),
			    m_storeShaderProgram->GetVValueLocation ()), false,
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


template<typename PropertySetter>
vtkSmartPointer<vtkFloatArray> ImageBasedAverage<PropertySetter>::getData (
    boost::shared_ptr<QGLFramebufferObject> framebuffer,
    const G3D::Rect2D& windowCoord, GLenum format) const
{
    VTK_CREATE(vtkFloatArray, attributes);
    vtkIdType numberOfPoints = windowCoord.width () * windowCoord.height ();
    size_t numberOfComponents = getNumberOfComponents (format);
    attributes->SetNumberOfComponents (numberOfComponents);
    attributes->SetNumberOfTuples (numberOfPoints);

    framebuffer->bind ();
    void* data = attributes->WriteVoidPointer (0, numberOfPoints);
    glReadPixels (
        windowCoord.x0 (), windowCoord.y0 (), 
        windowCoord.width (), windowCoord.height (), format, GL_FLOAT, data);
    framebuffer->release ();
    return attributes;
}

template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::save (
    vtkSmartPointer<vtkFloatArray> data, 
    const G3D::Rect2D& windowCoord, size_t components, float maxValue) const
{
    QImage image (windowCoord.width (), windowCoord.height (), 
                  QImage::Format_RGB32);
    ostringstream ostr;
    ostr << "images/data" << components << "-"
	 << setfill ('0') << setw (4) << GetWidgetGl ().GetTime ()  
         << ".png";
    QColor color;
    for (size_t x = 0; x < windowCoord.width (); ++x)
        for (size_t y = 0; y < windowCoord.height (); ++y)
        {
            float d = 0;
            if (components == 1)
                d = data->GetComponent (y * windowCoord.width () + x, 0);
            else if (components == 2)
            {
                // assume a vector, compute vector length
                float one = data->GetComponent (
                    y * windowCoord.width () + x, 0);
                float two = data->GetComponent (
                    y * windowCoord.width () + x, 1);
                d = sqrt (one * one + two * two);
            }
            color.setRgbF (d / maxValue, d / maxValue, d / maxValue);
            // QImage y value grows top - down, OpenGL grows bottom - up.
            image.setPixel (x, windowCoord.height () - 1 - y, color.rgb ());
        }
    image.save (ostr.str ().c_str ());
}


template<typename PropertySetter>
void ImageBasedAverage<PropertySetter>::save (
    FbosCountFbos fbo, const char* postfix, size_t timeStep, size_t subStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    // render to the debug buffer
    m_fbos.m_debug->bind ();
    ClearColorBuffer (Qt::white);
    rotateAndDisplay (minValue, maxValue, displayType, fbo, 
                      ViewingVolumeOperation::ENCLOSE2D);
    m_fbos.m_debug->release ();
    ostringstream ostr;
    ostr << "images/" 
	 << m_id << setfill ('0') << setw (4) << timeStep << "-" 
	 << setw (2) << subStep << postfix << ".png";
    m_fbos.m_debug->toImage ().save (ostr.str ().c_str ());
    WarnOnOpenGLError ("ImageBasedAverage::save");
}



// Template instantiations
//======================================================================
/// @cond
template class ImageBasedAverage<SetterVertexAttribute>;
template class ImageBasedAverage<SetterDeformation>;
template class ImageBasedAverage<SetterVelocity>;
template class ImageBasedAverage<SetterNop>;
/// @endcond
