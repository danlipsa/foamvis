/**
 * @file   TensorAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 *
 * Interface for the TensorAverage class
 */

#ifndef __TENSOR_AVERAGE_H__
#define __TENSOR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
class TensorDisplay;

/**
 * Calculate face average, min, max over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * current = (sum00,sum10,sum01,sum11) up to and including the current step
 * previous = (sum00, sum10, sum01, sum11) up to and including the previous step.
 * step = (m00, m10, m01, m11) or step = (0, 0, 0, 0) if there is no 
 * value for that pixel.
 */
class TensorAverage : public ImageBasedAverage<SetterDeformationTensor>
{
public:
    TensorAverage (const GLWidget& glWidget) :
	ImageBasedAverage<SetterDeformationTensor> (glWidget, "tensor")
    {
    }

    static void InitShaders ();

protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	const G3D::Rect2D& viewRect, GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, QGLFramebufferObject& fbo,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0);

private:
    static boost::shared_ptr<TensorDisplay> m_displayShaderProgram;
};

#endif //__TENSOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
