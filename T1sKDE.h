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
 * Gaussian 2D is a product of 1D Gaussians.
 * g_2D (x,y,s) = 1 / (2 * pi * s^2) * e ^ (0.5 * (x^2 + y^2) / s^2)
 */
class T1sKDE : public ScalarAverageTemplate<SetterNop>
{
public:
    static void InitShaders ();

public:    
    T1sKDE (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl);
    virtual void AverageInit ();

    float GetKernelSigma () const
    {
	return m_kernelSigma;
    }
    float GetKernelSigmaInBubbleDiameters () const;    
    void SetKernelSigmaInBubbleDiameters (float kernelSigmaInBubbleDiameters);
    float GetPeakHeight () const;

    size_t GetKernelTextureSize () const;

    bool IsKernelTextureShown () const
    {
	return m_kernelTextureShown;
    }
    void SetKernelTextureShown (bool kernelTextureShown)
    {
	m_kernelTextureShown = kernelTextureShown;
    }

    void DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
			     size_t subStep) const;
    void CacheData (boost::shared_ptr<AverageCache> averageCache) const;

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
    float m_kernelSigma;
    bool m_kernelTextureShown;
};

#endif //__T1S_KDE_H__

// Local Variables:
// mode: c++
// End:
