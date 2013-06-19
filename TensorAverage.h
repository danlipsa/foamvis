/**
 * @file   TensorAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 * @ingroup average
 * @brief Computes a pixel-based time-average of vector and tensor attributes
 */

#ifndef __TENSOR_AVERAGE_H__
#define __TENSOR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
#include "ShaderProgram.h"
#include "ViewSettings.h"

/**
 * @brief Shader used to display a 2D tensor as an ellipse
 */
class TensorDisplay : public ShaderProgram
{
public:
    TensorDisplay (const char* vert, const char* frag);
    void Bind (
	G3D::Vector2 gridTranslation, float gridCellLength, 
	float lineWidth, bool sameSize, bool colorMapped,
	float noiseStart, float noiseFrequency,
	float noiseAmplitude,
	float elipseSizeRatio, G3D::Rect2D enclosingRect,
	G3D::Vector2 rotationCenter, int countIndex, 
        float minValue, float maxValue,
	bool gridShown, bool clampingShown,
	bool gridCellCenterShown, float onePixelInObjectSpace, 
        bool glyphShown);

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
    int m_gridCellLenghtLocation;
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
    int m_countIndexLocation;
    int m_minValueLocation;
    int m_maxValueLocation;
    int m_overlayBarTexUnitLocation;
    int m_gridShownLocation;
    int m_clampingShownLocation;
    int m_gridCellCenterShownLocation;
    int m_onePixelInObjectSpaceLocation;
    int m_glyphShownLocation;
};

/**
 * @brief Computes a pixel-based time-average of vector and tensor attributes
 */
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
        const WidgetGl& widgetGl, AverageType::Enum averageType,
        WidgetGlFloatFunction sizeInitialRatio,
	ViewSettingsFloatFunction sizeRatio, 
        ViewSettingsFloatFunction lineWidthRatio,
        FramebufferObjects& countFbos, size_t countIndex);

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
    void SetGlyphShown (bool shown)
    {
        m_glyphShown = shown;
    }
    bool IsGlyphShown () const
    {
        return m_glyphShown;
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
	QwtDoubleInterval interval, StatisticsType::Enum displayType, 
	typename ImageBasedAverage<Setter>::FbosCountFbos srcFbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;

private:
    void calculateShaderParameters (
	G3D::Vector2 rotationCenter, 
	G3D::Vector2* gridTranslation, float* gridCellLength, 
        float* lineWidth, float* elipseSizeRatio, G3D::Rect2D* srcRect, 
	float* onePixelInObjectSpace) const;

protected:
    static boost::shared_ptr<TensorDisplay> m_displayShaderProgram;

private:
    bool m_gridShown;
    bool m_clampingShown;
    bool m_gridCellCenterShown;
    bool m_sameSize;
    bool m_colorMapped;
    bool m_glyphShown;
    float m_noiseAmplitude;
    float m_noiseStart;
    float m_noiseFrequency;
    WidgetGlFloatFunction m_sizeInitialRatio;
    ViewSettingsFloatFunction m_sizeRatio;
    ViewSettingsFloatFunction m_lineWidthRatio;
};


/**
 * @brief Computes a pixel-based time-average of tensor attributes
 */
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
