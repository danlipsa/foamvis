/**
 * @file   T1sPDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Interface for the T1sPDE class
 */

#ifndef __T1S_PDE_H__
#define __T1S_PDE_H__

#include "ScalarAverage.h"

/**
 * Calculate T1s average, over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * current = (sum,count,min,max) up to and including the current step
 * previous = (sum, count, min, max) up to and including the previous step.
 * step = (x, 1, x, x) for (sum, count, min, max) where x is the value for
 * one step. step = (0, 0, maxFloat, -maxFloat) if there is no 
 * value for that pixel.
 */
class T1sPDE : public ScalarAverage
{
public:
    static void InitShaders ();
    
    T1sPDE (const GLWidget& glWidget) :
	ScalarAverage (glWidget, "t1sPDE"),
	m_kernelIntervalMargin (KERNEL_INTERVAL_MARGIN.first),
	m_kernelSigma (KERNEL_SIGMA.first),
	m_kernelTextureSize (KERNEL_TEXTURE_SIZE.first)
    {
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);
    float GetKernelIntervalMargin () const
    {
	return m_kernelIntervalMargin;
    }
    void SetKernelIntervalMargin (float kernelIntervalMargin)
    {
	m_kernelIntervalMargin = kernelIntervalMargin;
    }
    float GetKernelSigma () const
    {
	return m_kernelSigma;
    }
    void SetKernelSigma (float kernelSigma)
    {
	m_kernelSigma = kernelSigma;
    }
    size_t GetKernelTextureSize () const
    {
	return m_kernelTextureSize;
    }
    void SetKernelTextureSize (size_t kernelTextureSize)
    {
	m_kernelTextureSize = kernelTextureSize;
    }

public:
    static const pair<size_t, size_t> KERNEL_TEXTURE_SIZE;
    static const pair<float, float> KERNEL_SIGMA;
    static const pair<float, float> KERNEL_INTERVAL_MARGIN;

protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	const G3D::Rect2D& viewRect, GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, TensorScalarFbo fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void writeStepValues (ViewNumber::Enum view, size_t timeStep);

private:
    void initKernel (ViewNumber::Enum viewNumber);

    boost::shared_ptr<QGLFramebufferObject> m_kernel;
    float m_kernelIntervalMargin;
    float m_kernelSigma;
    size_t m_kernelTextureSize;
};

#endif //__T1S_PDE_H__

// Local Variables:
// mode: c++
// End:
