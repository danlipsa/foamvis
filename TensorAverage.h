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
#include "ShaderProgram.h"
#include "ViewSettings.h"


class TensorDisplay : public ShaderProgram
{
public:
    TensorDisplay (const char* vert, const char* frag);
    void Bind (
	G3D::Vector2 gridTranslation, float cellLength, float lineWidth, 
	float elipseSizeRatio, G3D::Rect2D enclosingRect,
	G3D::Vector2 rotationCenter, 
	bool gridShown, bool gridCellCenterShown);

    GLint GetTensorAverageTexUnit ()
    {
	return 1;
    }
    GLint GetScalarAverageTexUnit ()
    {
	return 2;
    }

private:
    int m_gridTranslationLocation;
    int m_cellLengthLocation;
    int m_lineWidthLocation;
    int m_ellipseSizeRatioLocation;
    int m_enclosingRectLowLocation;
    int m_enclosingRectHighLocation;
    int m_rotationCenterLocation;
    int m_tensorAverageTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
    int m_gridShownLocation;
    int m_gridCellCenterShownLocation;
};

template<typename Setter>
class TensorAverageTemplate : public ImageBasedAverage<Setter>
{
public:
    typedef float (GLWidget::*ParameterView) (
	ViewNumber::Enum viewNumber) const;
    typedef float (ViewSettings::*Parameter) () const;    

public:
    TensorAverageTemplate (
	const GLWidget& glWidget, ParameterView sizeInitialRatio,
	Parameter sizeRatio, Parameter lineWidthRatio,
	FramebufferObjects& scalarAverageFbos) :
	ImageBasedAverage<Setter> (
	    glWidget, "tensor", QColor (0, 0, 0, 0), scalarAverageFbos),
	m_gridShown (false),
	m_gridCellCenterShown (false),
	m_sizeInitialRatio (sizeInitialRatio),
	m_sizeRatio (sizeRatio),
	m_lineWidthRatio (lineWidthRatio)
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
    void SetGridCellCenterShown (bool shown)
    {
	m_gridCellCenterShown = shown;
    }
    bool IsGridCellCenterShown ()
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

protected:
    static boost::shared_ptr<TensorDisplay> m_displayShaderProgram;

private:
    bool m_gridShown;
    bool m_gridCellCenterShown;    
    ParameterView m_sizeInitialRatio;
    Parameter m_sizeRatio;
    Parameter m_lineWidthRatio;
};

class TensorAverage : public TensorAverageTemplate<SetterDeformation>
{
public:
    TensorAverage (const GLWidget& glWidget,
		   FramebufferObjects& scalarAverageFbos);
};

#endif //__TENSOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
