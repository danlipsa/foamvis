/**
 * @file   T1sKDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Interface for the T1sKDE class
 */

#ifndef __T1S_KDE_H__
#define __T1S_KDE_H__

#include "ScalarAverage.h"

class GaussianInitShaderProgram;
class GaussianStoreShaderProgram;

/**
 * Calculate T1s average, over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * current = (sum,count,min,max) up to and including the current step
 * previous = (sum, count, min, max) up to and including the previous step.
 * step = (x, 1, x, x) for (sum, count, min, max) where x is the value for
 * one step. step = (0, 0, maxFloat, -maxFloat) if there is no 
 * value for that pixel.
 */
class T1sKDE : public ScalarAverageTemplate<SetterNop>
{
public:
    static void InitShaders ();

public:    
    T1sKDE (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl);
    virtual void AverageInit ();

    float GetKernelIntervalPerPixel () const
    {
	return m_kernelIntervalPerPixel;
    }
    void SetKernelIntervalPerPixel (float kernelIntervalMargin)
    {
	m_kernelIntervalPerPixel = kernelIntervalMargin;
	initKernel ();
    }

    float GetKernelSigma () const
    {
	return m_kernelSigma;
    }
    void SetKernelSigma (float kernelSigma)
    {
	m_kernelSigma = kernelSigma;
	initKernel ();
    }

    size_t GetKernelTextureSize () const
    {
	return m_kernelTextureSize;
    }
    void SetKernelTextureSize (size_t kernelTextureSize);

    bool IsKernelTextureSizeShown () const
    {
	return m_kernelTextureSizeShown;
    }
    void SetKernelTextureSizeShown (bool kernelTextureSizeShown)
    {
	m_kernelTextureSizeShown = kernelTextureSizeShown;
    }

    void DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
			     size_t subStep) const;

public:
    static const pair<size_t, size_t> KERNEL_TEXTURE_SIZE;
    static const pair<float, float> KERNEL_SIGMA;
    static const pair<float, float> KERNEL_INTERVAL_PER_PIXEL;

protected:
    virtual void writeStepValues (
        ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep);
    virtual size_t getStepSize (size_t timeStep) const;

private:
    void initKernel ();


private:
    static boost::shared_ptr<GaussianInitShaderProgram
			     > m_gaussianInitShaderProgram;
    static boost::shared_ptr<GaussianStoreShaderProgram
			     > m_gaussianStoreShaderProgram;


    boost::shared_ptr<QGLFramebufferObject> m_kernel;
    float m_kernelIntervalPerPixel;
    float m_kernelSigma;
    size_t m_kernelTextureSize;
    bool m_kernelTextureSizeShown;
};

#endif //__T1S_KDE_H__

// Local Variables:
// mode: c++
// End:
