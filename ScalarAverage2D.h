/**
 * @file   ScalarAverage2D.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 *
 * Interface for the ScalarAverage2D class
 */

#ifndef __SCALAR_AVERAGE_2D_H__
#define __SCALAR_AVERAGE_2D_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
class ScalarDisplay;
class AverageCache;

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
class ScalarAverage2DTemplate : public ImageBasedAverage<PropertySetter>
{
public:
    static void InitShaders ();

    ScalarAverage2DTemplate (ViewNumber::Enum viewNumber, 
                             const WidgetGl& widgetGl, AverageType::Enum type, 
                             QColor stepClearColor);

protected:
    virtual void rotateAndDisplay (
	QwtDoubleInterval interval,
	StatisticsType::Enum displayType, 
	typename ImageBasedAverage<PropertySetter>::FbosCountFbos fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

protected:
    static boost::shared_ptr<ScalarDisplay> m_displayShaderProgram;
    vtkSmartPointer<vtkImageData> getData (AverageType::Enum averageType) const;
};

class ScalarAverage2D : public ScalarAverage2DTemplate<SetterVertexAttribute>
{
public:
    ScalarAverage2D (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl) :
	ScalarAverage2DTemplate<SetterVertexAttribute> (
	    viewNumber, widgetGl, AverageType::SCALAR, QColor (0, 0, 0, 0))
    {
    }
};

#endif //__SCALAR_AVERAGE_2D_H__

// Local Variables:
// mode: c++
// End:
