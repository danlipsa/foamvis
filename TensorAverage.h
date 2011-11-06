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

template<typename Setter>
class TensorAverageTemplate : public ImageBasedAverage<Setter>
{
public:
    TensorAverageTemplate (const GLWidget& glWidget, 
			   FramebufferObjects& scalarAverageFbos) :
	ImageBasedAverage<Setter> (
	    glWidget, "tensor", QColor (0, 0, 0, 0), scalarAverageFbos),
	m_gridShown (false),
	m_gridCellCenterShown (false)
    {
    }
    static void InitShaders ();
    void SetDeformationGridShown (bool shown)
    {
	m_gridShown = shown;
    }
    bool IsDeformationGridShown () const
    {
	return m_gridShown;
    }

    void SetDeformationGridCellCenterShown (bool shown)
    {
	m_gridCellCenterShown = shown;
    }

    bool IsDeformationGridCellCenterShown ()
    {
	return m_gridCellCenterShown;
    }

protected:
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, 
	typename ImageBasedAverage<Setter>::TensorScalarFbo srcFbo,
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
    bool m_gridShown;
    bool m_gridCellCenterShown;
};

class TensorAverage : public TensorAverageTemplate<SetterDeformation>
{
public:
    TensorAverage (const GLWidget& glWidget, 
		   FramebufferObjects& scalarAverageFbos) :
	TensorAverageTemplate<SetterDeformation> (glWidget, scalarAverageFbos)
    {
    }
};

#endif //__TENSOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
