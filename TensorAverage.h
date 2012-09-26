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
	G3D::Vector2 gridTranslation, float cellLength, 
	float lineWidth, bool sameSize, bool colorMapped,
	float noiseStart, float noiseFrequency,
	float noiseAmplitude,
	float elipseSizeRatio, G3D::Rect2D enclosingRect,
	G3D::Vector2 rotationCenter, float minValue, float maxValue,
	bool gridShown, bool clampingShown,
	bool gridCellCenterShown, float onePixelInObjectSpace);

    GLint GetTensorAverageTexUnit ()
    {
	return 1;
    }
    GLint GetScalarAverageTexUnit ()
    {
	return 2;
    }
    GLint GetOverlayBarTexUnit ()
    {
	return 0;
    }

private:
    int m_gridTranslationLocation;
    int m_cellLengthLocation;
    int m_lineWidthLocation;
    int m_sameSizeLocation;
    int m_colorMappedLocation;
    int m_noiseStartLocation;
    int m_noiseFrequencyLocation;
    int m_noiseAmplitudeLocation;
    int m_ellipseSizeRatioLocation;
    int m_enclosingRectLowLocation;
    int m_enclosingRectHighLocation;
    int m_rotationCenterLocation;
    int m_tensorAverageTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
    int m_minValueLocation;
    int m_maxValueLocation;
    int m_overlayBarTexUnitLocation;
    int m_gridShownLocation;
    int m_clampingShownLocation;
    int m_gridCellCenterShownLocation;
    int m_onePixelInObjectSpaceLocation;
};

template<typename Setter>
class TensorAverageTemplate : public ImageBasedAverage<Setter>
{
public:
    typedef float (WidgetGl::*WidgetGlFloatFunction) (
	ViewNumber::Enum viewNumber) const;
    typedef float (ViewSettings::*ViewSettingsFloatFunction) () const;

public:
    TensorAverageTemplate (
        ViewNumber::Enum viewNumber,
        const WidgetGl& widgetGl, WidgetGlFloatFunction sizeInitialRatio,
	ViewSettingsFloatFunction sizeRatio, 
        ViewSettingsFloatFunction lineWidthRatio,
        FramebufferObjects& countFbos) :

	ImageBasedAverage<Setter> (viewNumber,
	    widgetGl, "tensor", QColor (0, 0, 0, 0), countFbos),
	m_gridShown (false),
	m_clampingShown (false),
	m_gridCellCenterShown (false),
	m_sameSize (false),
	m_colorMapped(false),
	
        /*m_noiseAmplitude (10.0),*/ m_noiseAmplitude (0),
	m_noiseStart (0.5),
	m_noiseFrequency (317),
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
    void SetClampingShown (bool shown)
    {
	m_clampingShown = shown;
    }
    bool IsClampingShown () const
    {
	return m_clampingShown;
    }
    void SetGridCellCenterShown (bool shown)
    {
	m_gridCellCenterShown = shown;
    }
    bool IsGridCellCenterShown () const
    {
	return m_gridCellCenterShown;
    }
    void SetSameSize (bool sameSize)
    {
	m_sameSize = sameSize;
    }
    bool IsSameSize () const
    {
	return m_sameSize;
    }
    void SetColorMapped (bool colorMapped)
    {
	m_colorMapped = colorMapped;
    }
    bool IsColorMapped () const
    {
	return m_colorMapped;
    }
    void SetNoiseStart (float noiseStart)
    {
	m_noiseStart = noiseStart;
    }
    void SetNoiseFrequency (float noiseFrequency)
    {
	m_noiseFrequency = noiseFrequency;
    }
    void SetNoiseAmplitude (float noiseAmplitude)
    {
	m_noiseAmplitude = noiseAmplitude;
    }
    

protected:
    virtual void rotateAndDisplay (
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, 
	typename ImageBasedAverage<Setter>::TensorScalarFbo srcFbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

private:
    void calculateShaderParameters (
	G3D::Vector2 rotationCenter, 
	G3D::Vector2* gridTranslation, float* cellLength, float* lineWidth, 	
	float* elipseSizeRatio, G3D::Rect2D* srcRect, 
	float* onePixelInObjectSpace) const;

protected:
    static boost::shared_ptr<TensorDisplay> m_displayShaderProgram;

private:
    bool m_gridShown;
    bool m_clampingShown;
    bool m_gridCellCenterShown;
    bool m_sameSize;
    bool m_colorMapped;
    float m_noiseAmplitude;
    float m_noiseStart;
    float m_noiseFrequency;
    WidgetGlFloatFunction m_sizeInitialRatio;
    ViewSettingsFloatFunction m_sizeRatio;
    ViewSettingsFloatFunction m_lineWidthRatio;
};

class TensorAverage : public TensorAverageTemplate<SetterDeformation>
{
public:
    TensorAverage (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl,
		   FramebufferObjects& countFbos);
};

#endif //__TENSOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
