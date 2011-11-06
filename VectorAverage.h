/**
 * @file   VectorAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 *
 * Interface for the VectorAverage class
 */

#ifndef __VECTOR_AVERAGE_H__
#define __VECTOR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
class ScalarAverage;
class VectorDisplay;

class VectorAverage : public ImageBasedAverage<SetterVelocity>
{
public:
    VectorAverage (const GLWidget& glWidget, 
		   FramebufferObjects& scalarAverageFbos) :
	ImageBasedAverage<SetterVelocity> (
	    glWidget, "vector", QColor (0, 0, 0, 0), scalarAverageFbos),
	m_gridShown (false)
    {
    }
    static void InitShaders ();
    void SetGridShown (bool shown)
    {
	m_gridShown = shown;
    }
    bool IsGridShown () const
    {
	return m_gridShown;
    }

protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, TensorScalarFbo srcFbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

private:
    void calculateShaderParameters (
	ViewNumber::Enum viewNumber, G3D::Vector2 rotationCenter, 
	float angleDegrees,
	G3D::Vector2* gridTranslation, float* cellLength, float* lineWidth, 
	float* elipseSizeRatio, G3D::Rect2D* srcRect) const;

private:
    static boost::shared_ptr<VectorDisplay> m_displayShaderProgram;
    bool m_gridShown;
};

#endif //__VECTOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
