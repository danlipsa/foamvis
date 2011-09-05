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

const size_t T1sPDE::KERNEL_TEXTURE_SIZE = 64;
const float T1sPDE::KERNEL_INTERVAL_MARGIN = 5.0;
const float T1sPDE::KERNEL_SIGMA = 1.0;

void T1sPDE::AverageInit (ViewNumber::Enum viewNumber)
{
    ScalarAverage::AverageInit (viewNumber);
    initKernel (viewNumber);
}


void T1sPDE::initKernel (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetGLWidget ().GetViewSettings (viewNumber);
    size_t newSize = KERNEL_TEXTURE_SIZE * vs.GetScaleRatio ();
    QSize size (newSize, newSize);
    cdbg << newSize << endl;
    m_kernel.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    RuntimeAssert (m_kernel->isValid (), 
		   "Framebuffer initialization failed:" + GetId ());
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
