/**
 * @file   T1sPDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Implementation for the T1sPDE class 
 */

#include "Debug.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "T1sPDE.h"
#include "ViewSettings.h"

// Private classes/functions
// ======================================================================


// T1sPDE Methods and static fields
// ======================================================================

const pair<size_t, size_t> T1sPDE::KERNEL_TEXTURE_SIZE = 
    pair<size_t, size_t> (32, 128);
const pair<float, float> T1sPDE::KERNEL_INTERVAL_MARGIN = 
    pair<float, float> (5.0, 10.0);
const pair<float, float> T1sPDE::KERNEL_SIGMA = pair<float, float> (1.0, 5.0);

void T1sPDE::AverageInit (ViewNumber::Enum viewNumber)
{
    ScalarAverage::AverageInit (viewNumber);
    initKernel (viewNumber);
}


// Interactive Visualization of Streaming Data with Kernel Density Estimation
// Ove Daae Lampe and Helwig Hauser
// h: bandwidth is equal with standard deviation
void T1sPDE::initKernel (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetGLWidget ().GetViewSettings (viewNumber);
    QSize size (m_kernelTextureSize, m_kernelTextureSize);
    m_kernel.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    RuntimeAssert (m_kernel->isValid (), 
		   "Framebuffer initialization failed:" + GetId ());
/*
    const G3D::Rect2D destRect = EncloseRotation (
	GetGLWidget ().GetViewRect (viewNumber));
    fbo->bind ();
    m_initShaderProgram->Bind ();
    ActivateShader (destRect - destRect.x0y0 (),
	ViewingVolumeOperation::ENCLOSE2D);
    m_initShaderProgram->release ();
    fbo->release ();
*/
}

void T1sPDE::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    const G3D::Rect2D& destRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, FramebufferObjectPair srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
}

void T1sPDE::writeStepValues (ViewNumber::Enum view, size_t timeStep)
{
}
