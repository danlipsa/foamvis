/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 *
 * Interface for the ScalarAverage class
 */

#ifndef __SCALAR_AVERAGE_H__
#define __SCALAR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
class ScalarDisplay;

/**
 * Calculate face average, min, max over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * current = (sum,count,min,max) up to and including the current step
 * previous = (sum, count, min, max) up to and including the previous step.
 * step = (x, 1, x, x) for (sum, count, min, max) where x is the value for
 * one step. step = (0, 0, maxFloat, -maxFloat) if there is no 
 * value for that pixel.
 */
class ScalarAverage : public ImageBasedAverage<SetterVertexAttribute>
{
public:
    static void InitShaders ();

    ScalarAverage (const GLWidget& glWidget, string id = "scalar") :
	ImageBasedAverage<SetterVertexAttribute> (glWidget, id, m_fbos)
    {
    }

protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, TensorScalarFbo fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

private:
    static boost::shared_ptr<ScalarDisplay> m_displayShaderProgram;
};

#endif //__SCALAR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
