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

/**
 * Calculate face average, min, max over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * Average is implemented by first calculating the sum and then dividing by
 * the number of elements in the sum. The sum is calculated in 3 steps:
 * 1. step = draw current foam using attribute values instead of colors
 * 2. current = previous + step
 * 3. previous = current
 *
 * The reason for this type of implementation is that OpenGL cannot
 * read and write to the same buffer in the same step.
 */
class ScalarAverage : public ImageBasedAverage
{
public:
    ScalarAverage (const GLWidget& glWidget) :
    ImageBasedAverage (glWidget)
    {
    }

    static void InitShaders ();

protected:
    virtual void display (
	const G3D::Rect2D& viewRect, 
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo);
    virtual void displayAndRotate (
	const G3D::Rect2D& viewRect, GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, QGLFramebufferObject& fbo,
	G3D::Vector2 rotationCenter, float angleDegrees);

private:
    static boost::shared_ptr<DisplayShaderProgram> m_displayShaderProgram;
};

#endif //__SCALAR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
