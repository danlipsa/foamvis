/**
 * @file   T1KDE2D.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Interface for the T1KDE2D class
 */

#ifndef __T1_KDE_2D_H__
#define __T1_KDE_2D_H__

#include "ScalarAverage2D.h"

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
class T1KDE2D : public ScalarAverage2DTemplate<SetterNop>
{
public:
    static void InitShaders ();

public:    
    T1KDE2D (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl);
    virtual void AverageInit ();

    float GetPeakHeight () const;

    size_t GetKernelTextureSize () const;

    void DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
			     size_t subStep) const;
    void CacheData (boost::shared_ptr<AverageCache> averageCache) const;
    void InitKernel ();

protected:
    virtual void writeStepValues (
        ViewNumber::Enum viewNumber, size_t timeStep, size_t subStep);
    virtual size_t getStepSize (size_t timeStep) const;

private:
    float getKernelSigma () const;

private:
    static boost::shared_ptr<GaussianInitShaderProgram
			     > m_gaussianInitShaderProgram;
    static boost::shared_ptr<GaussianStoreShaderProgram
			     > m_gaussianStoreShaderProgram;


    boost::shared_ptr<QGLFramebufferObject> m_kernel;
};

#endif //__T1_KDE_2D_H__

// Local Variables:
// mode: c++
// End:
