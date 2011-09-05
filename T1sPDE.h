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
	ScalarAverage (glWidget, "t1sPDE")
    {
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);



protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	const G3D::Rect2D& viewRect, GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, FramebufferObjectPair fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void writeStepValues (ViewNumber::Enum view, size_t timeStep);

private:
    void initKernel (ViewNumber::Enum viewNumber);

private:
    static const size_t KERNEL_TEXTURE_SIZE;
    static const float KERNEL_SIGMA;
    static const float KERNEL_INTERVAL_MARGIN;

    boost::shared_ptr<QGLFramebufferObject> m_kernel;
};

#endif //__T1S_PDE_H__

// Local Variables:
// mode: c++
// End:
