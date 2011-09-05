/**
 * @file   T1sPDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Implementation for the T1sPDE class 
 */

#include "Debug.h"
#include "T1sPDE.h"

// Private classes/functions
// ======================================================================


// T1sPDE Methods and static fields
// ======================================================================

const size_t T1sPDE::KERNEL_TEXTURE_SIZE = 64;

void T1sPDE::Init (ViewNumber::Enum viewNumber)
{
    ScalarAverage::Init (viewNumber);
    QSize size (KERNEL_TEXTURE_SIZE, KERNEL_TEXTURE_SIZE);
    m_kernel.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    RuntimeAssert (m_kernel->isValid (), 
		   "Framebuffer initialization failed:" + GetId ());
    initKernel ();
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


// Interactive Visualization of Streaming Data with Kernel Density Estimation
// Ove Daae Lampe and Helwig Hauser
// h: bandwidth is equal with standard deviation
/*
void GLWidget::calculateGaussian (
    boost::array< boost::array<float, GAUSSIAN_TEXTURE_SIZE>, 
    GAUSSIAN_TEXTURE_SIZE>, float h, float GAUSSIAN_INTERVAL)
{
    
}
*/
