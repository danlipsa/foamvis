/**
 * @file   T1sPDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Implementation for the T1sPDE class 
 */

#include "AverageShaders.h"
#include "Debug.h"
#include "DebugStream.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "ScalarDisplay.h"
#include "ShaderProgram.h"
#include "T1sPDE.h"
#include "Utils.h"
#include "ViewSettings.h"

// Private classes/functions
// ======================================================================

class GaussianInitShaderProgram : public ShaderProgram
{
public:
    GaussianInitShaderProgram (const char* frag);
    void Bind (float kernelSigma, float kernelIntervalMargin);
protected:
    float m_sigmaLocation;
    float m_intervalMarginLocation;
};

GaussianInitShaderProgram::GaussianInitShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_intervalMarginLocation = uniformLocation("m_intervalMargin");
    RuntimeAssert (m_intervalMarginLocation != -1, "Invalid location: m_intervalMargin");

    m_sigmaLocation = uniformLocation("u_sigma");
    RuntimeAssert (m_sigmaLocation != -1, "Invalid location: u_sigma");
}

void GaussianInitShaderProgram::Bind (float sigma,
				      float intervalMargin)
{
    ShaderProgram::Bind ();
    setUniformValue (m_sigmaLocation, sigma);
    setUniformValue (m_intervalMarginLocation, intervalMargin);
}


class GaussianStoreShaderProgram : public ShaderProgram
{
public:
    GaussianStoreShaderProgram (const char* frag);
    GLint GetGaussianTexUnit ()
    {
	return 1;
    }
    void Bind ();
protected:
    int m_gaussianTexUnitLocation;
};

GaussianStoreShaderProgram::GaussianStoreShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_gaussianTexUnitLocation = uniformLocation("u_gaussianTexUnit");
}

void GaussianStoreShaderProgram::Bind ()
{
    ShaderProgram::Bind ();
    setUniformValue (m_gaussianTexUnitLocation, GetGaussianTexUnit ());
}


// T1sPDE Methods and static fields
// ======================================================================

const pair<size_t, size_t> T1sPDE::KERNEL_TEXTURE_SIZE = 
    pair<size_t, size_t> (16, 128);
const pair<float, float> T1sPDE::KERNEL_INTERVAL_PER_PIXEL = 
    pair<float, float> (5.0/16.0, 10.0/16.0);
const pair<float, float> T1sPDE::KERNEL_SIGMA = pair<float, float> (1.0, 5.0);
boost::shared_ptr<GaussianInitShaderProgram> T1sPDE::m_gaussianInitShaderProgram;
boost::shared_ptr<GaussianStoreShaderProgram
		  > T1sPDE::m_gaussianStoreShaderProgram;


void T1sPDE::InitShaders ()
{
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("T1sPDEInit.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
    m_gaussianInitShaderProgram.reset (new GaussianInitShaderProgram
				       (RESOURCE ("GaussianInit.frag")));
    m_gaussianStoreShaderProgram.reset (new GaussianStoreShaderProgram
				       (RESOURCE ("GaussianStore.frag")));
}

T1sPDE::T1sPDE (const GLWidget& glWidget) :
    ScalarAverageTemplate<SetterNop> (glWidget, "t1sPDE", QColor (0, 255, 0, 0)),
    m_kernelIntervalPerPixel (KERNEL_INTERVAL_PER_PIXEL.first),
    m_kernelSigma (KERNEL_SIGMA.first),
    m_kernelTextureSize (KERNEL_TEXTURE_SIZE.first),
    m_kernelTextureSizeShown (false)
{
}

void T1sPDE::AverageInit (ViewNumber::Enum viewNumber)
{
    WarnOnOpenGLError ("a - T1sPDE::AverageInit");
    ScalarAverageTemplate<SetterNop>::AverageInit (viewNumber);
    initKernel ();
    WarnOnOpenGLError ("b - T1sPDE::AverageInit");
}


// Interactive Visualization of Streaming Data with Kernel Density Estimation
// Ove Daae Lampe and Helwig Hauser
// h: bandwidth is equal with standard deviation
void T1sPDE::initKernel ()
{
    QSize size (m_kernelTextureSize, m_kernelTextureSize);
    m_kernel.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    RuntimeAssert (m_kernel->isValid (), 
		   "Framebuffer initialization failed:" + GetId ());
    m_kernel->bind ();
    m_gaussianInitShaderProgram->Bind (
	m_kernelSigma, m_kernelIntervalPerPixel * m_kernelTextureSize);
    ActivateShader (
	G3D::Rect2D (G3D::Vector2 (m_kernelTextureSize, m_kernelTextureSize)));
    m_gaussianInitShaderProgram->release ();
    m_kernel->release ();
}

void T1sPDE::SetKernelTextureSize (size_t kernelTextureSize)
{
    m_kernelTextureSize = kernelTextureSize;
    initKernel ();
}


void T1sPDE::writeStepValues (ViewNumber::Enum viewNumber, size_t timeStep, 
			      size_t subStep)
{
    WarnOnOpenGLError ("a - T1sPDE::writeStepValues");
    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_gaussianStoreShaderProgram->GetGaussianTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_kernel->texture ());
    m_gaussianStoreShaderProgram->Bind ();
    GetGLWidget ().DisplayT1Quad (viewNumber, timeStep, subStep);
    // activate texture unit 0
    m_gaussianStoreShaderProgram->release ();
    glActiveTexture (GL_TEXTURE0);    
    WarnOnOpenGLError ("b - T1sPDE::writeStepValues");
}

void T1sPDE::DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
				 size_t subStep) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_POLYGON_BIT);
    glColor (
	GetGLWidget ().GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT, GL_LINE);
    GetGLWidget ().DisplayT1Quad (viewNumber, timeStep, subStep);
    glPopAttrib ();
}

size_t T1sPDE::getStepSize (size_t timeStep) const
{
    return GetGLWidget ().GetFoamAlongTime ().GetT1s (timeStep).size ();
}
