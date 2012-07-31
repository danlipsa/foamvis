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
template<typename PropertySetter>
class ScalarAverageTemplate : public ImageBasedAverage<PropertySetter>
{
public:
    static void InitShaders ();

    ScalarAverageTemplate (ViewNumber::Enum viewNumber, 
			   const WidgetGl& widgetGl, string id, 
			   QColor stepClearColor) :
	ImageBasedAverage<PropertySetter> (
	    viewNumber, widgetGl, id, stepClearColor, this->m_fbos)
    {
    }

protected:
    virtual void rotateAndDisplay (
	GLfloat minValue, GLfloat maxValue,
	ComputationType::Enum displayType, 
	typename ImageBasedAverage<PropertySetter>::TensorScalarFbo fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

protected:
    static boost::shared_ptr<ScalarDisplay> m_displayShaderProgram;
};

class ScalarAverage : public ScalarAverageTemplate<SetterVertexAttribute>
{
public:
    ScalarAverage (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl) :
	ScalarAverageTemplate<SetterVertexAttribute> (
	    viewNumber, widgetGl, "scalar", QColor (0, 0, 0, 0))
    {
    }
};

#endif //__SCALAR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
