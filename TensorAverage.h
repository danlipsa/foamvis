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
class ScalarAverage;
class TensorDisplay;

class TensorAverage : public ImageBasedAverage<SetterDeformationTensor>
{
public:
    TensorAverage (const GLWidget& glWidget, 
		   FramebufferObjects& scalarAverageFbos) :
	ImageBasedAverage<SetterDeformationTensor> (
	    glWidget, "tensor", QColor (0, 0, 0, 0), scalarAverageFbos),
	m_deformationGridShown (false),
	m_deformationGridCellCenterShown (false)
    {
    }
    static void InitShaders ();
    void SetDeformationGridShown (bool shown)
    {
	m_deformationGridShown = shown;
    }
    bool IsDeformationGridShown () const
    {
	return m_deformationGridShown;
    }

    void SetDeformationGridCellCenterShown (bool shown)
    {
	m_deformationGridCellCenterShown = shown;
    }

    bool IsDeformationGridCellCenterShown ()
    {
	return m_deformationGridCellCenterShown;
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
    static boost::shared_ptr<TensorDisplay> m_displayShaderProgram;
    bool m_deformationGridShown;
    bool m_deformationGridCellCenterShown;
};

#endif //__TENSOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
