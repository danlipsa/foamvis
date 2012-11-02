/**
 * @file   TensorAverage.cpp
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the TensorAverage class 
 *
 */

#include "AverageShaders.h"
#include "DebugStream.h"
#include "Foam.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "Simulation.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"


// Private classes/functions
// ======================================================================

TensorDisplay::TensorDisplay (const char* vert, const char* frag) :
    ShaderProgram (vert, frag)
{
    m_gridTranslationLocation = uniformLocation ("u_gridTranslationE");
    m_gridCellLenghtLocation = uniformLocation ("u_gridCellLength");
    m_lineWidthLocation = uniformLocation ("u_lineWidth");
    m_sameSizeLocation = uniformLocation ("u_sameSize");
    m_colorMappedLocation = uniformLocation ("u_colorMapped");
    m_noiseStartLocation = uniformLocation ("u_noiseStart");
    m_noiseFrequencyLocation = uniformLocation ("u_noiseFrequency");
    m_noiseAmplitudeLocation = uniformLocation ("u_noiseAmplitude");
    m_ellipseSizeRatioLocation = uniformLocation ("u_sizeRatio");
    m_enclosingRectLowLocation = uniformLocation ("u_enclosingRect.m_low");
    m_enclosingRectHighLocation = uniformLocation ("u_enclosingRect.m_high");
    m_rotationCenterLocation = uniformLocation ("u_rotationCenter");
    m_tensorAverageTexUnitLocation = uniformLocation("u_tensorAverageTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation ("u_scalarAverageTexUnit");
    m_countIndexLocation = uniformLocation ("u_countIndex");
    m_minValueLocation = uniformLocation ("u_minValue");
    m_maxValueLocation = uniformLocation ("u_maxValue");
    m_overlayBarTexUnitLocation = uniformLocation ("u_overlayBarTexUnit");
    m_gridShownLocation = uniformLocation ("u_gridShown");
    m_clampingShownLocation = uniformLocation ("u_clampingShown");
    m_gridCellCenterShownLocation = 
	uniformLocation ("u_gridCellCenterShown");
    m_onePixelInObjectSpaceLocation = 
	uniformLocation ("u_onePixelInObjectSpace");
    m_glyphShownLocation = uniformLocation ("u_glyphShown");
}

void TensorDisplay::Bind (
    G3D::Vector2 gridTranslation, float gridCellLength, float lineWidth,
    bool sameSize, bool colorMapped,
    float noiseStart, float noiseFrequency, float noiseAmplitude,
    float ellipseSizeRatio, G3D::Rect2D enclosingRect, 
    G3D::Vector2 rotationCenter, int countIndex, float minValue, float maxValue,
    bool gridShown, bool clampingShown, bool gridCellCenterShown,
    float onePixelInObjectSpace, bool glyphShown)
{
    ShaderProgram::Bind ();
    setUniformValue (
	m_gridTranslationLocation, gridTranslation[0], gridTranslation[1]);
    setUniformValue (m_gridCellLenghtLocation, gridCellLength);
    setUniformValue (m_lineWidthLocation, lineWidth);
    setUniformValue (m_sameSizeLocation, sameSize);
    setUniformValue (m_colorMappedLocation, colorMapped);
    setUniformValue (m_noiseStartLocation, noiseStart);
    setUniformValue (m_noiseFrequencyLocation, noiseFrequency);
    setUniformValue (m_noiseAmplitudeLocation, noiseAmplitude);
    setUniformValue (m_ellipseSizeRatioLocation, ellipseSizeRatio);
    setUniformValue (m_enclosingRectLowLocation, 
		     enclosingRect.x0 (), enclosingRect.y0 ());
    setUniformValue (m_enclosingRectHighLocation, 
		     enclosingRect.x1 (), enclosingRect.y1 ());
    setUniformValue (
	m_rotationCenterLocation, rotationCenter[0], rotationCenter[1]);
    setUniformValue (
	m_tensorAverageTexUnitLocation, GetTensorAverageTexUnit ());
    setUniformValue (
	m_scalarAverageTexUnitLocation, GetScalarAverageTexUnit ());
    setUniformValue (m_countIndexLocation, countIndex);
    setUniformValue (m_minValueLocation, minValue);
    setUniformValue (m_maxValueLocation, maxValue);
    setUniformValue (
	m_overlayBarTexUnitLocation, GetOverlayBarTexUnit ());
    setUniformValue (m_gridShownLocation, gridShown);
    setUniformValue (m_clampingShownLocation, clampingShown);
    setUniformValue (m_gridCellCenterShownLocation, 
		     gridCellCenterShown);
    setUniformValue (m_onePixelInObjectSpaceLocation, onePixelInObjectSpace);
    setUniformValue (m_glyphShownLocation, glyphShown);
}


// TensorAverage Methods
// ======================================================================
// @todo Implement clamping for tensor size: Initially, render the
// ellipse so that the maximum is fully rendered. Then, when
// increasing the size, keep the maximum size fixed.

template<typename Setter>
boost::shared_ptr<TensorDisplay> TensorAverageTemplate<
    Setter>::m_displayShaderProgram;

template<typename Setter>
void TensorAverageTemplate<Setter>::InitShaders ()
{
    cdbg << "==== TensorAverageTemplate ====" << endl;
    ImageBasedAverage<Setter>::m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("TensorInit.frag")));
    ImageBasedAverage<Setter>::m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("TensorStore.vert"), RESOURCE("TensorStore.frag")));
    ImageBasedAverage<Setter>::m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorAdd.frag")));
    ImageBasedAverage<Setter>::m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorRemove.frag")));
    m_displayShaderProgram.reset (
	new TensorDisplay (RESOURCE("TensorDisplay.vert"),
			   RESOURCE("TensorDisplay.frag")));
}


template<typename Setter>
TensorAverageTemplate<Setter>::TensorAverageTemplate (
    ViewNumber::Enum viewNumber,
    const WidgetGl& widgetGl, string id,
    WidgetGlFloatFunction sizeInitialRatio,
    ViewSettingsFloatFunction sizeRatio, 
    ViewSettingsFloatFunction lineWidthRatio,
    FramebufferObjects& countFbos, size_t countIndex) :
    
    ImageBasedAverage<Setter> (
        viewNumber,
        widgetGl, id, QColor (0, 0, 0, 0), countFbos, countIndex),
    m_gridShown (false),
    m_clampingShown (false),
    m_gridCellCenterShown (false),
    m_sameSize (false),
	m_colorMapped(false),
    m_glyphShown (true),
    
    /*m_noiseAmplitude (10.0),*/ m_noiseAmplitude (0),
    m_noiseStart (0.5),
    m_noiseFrequency (317),
    m_sizeInitialRatio (sizeInitialRatio),
    m_sizeRatio (sizeRatio),
    m_lineWidthRatio (lineWidthRatio)
{
}

template<typename Setter>
void TensorAverageTemplate<Setter>::rotateAndDisplay (
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, 
    typename ImageBasedAverage<Setter>::FbosCountFbos srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)minValue;(void)maxValue;(void)displayType;

    const WidgetGl& widgetGl = this->GetWidgetGl ();
    G3D::Vector2 gridTranslation;float gridCellLength; float lineWidth;
    float sizeRatio;G3D::Rect2D enclosingRect;float onePixelInObjectSpace;
    pair<float,float> minMax = 
	widgetGl.GetVelocityMagnitudeRange (this->GetViewNumber ());
    calculateShaderParameters (
	rotationCenter, &gridTranslation, 
	&gridCellLength, &lineWidth, &sizeRatio, &enclosingRect, 
	&onePixelInObjectSpace);
    m_displayShaderProgram->Bind (
	gridTranslation, gridCellLength, lineWidth, m_sameSize, m_colorMapped,
	m_noiseStart, m_noiseFrequency,
	m_noiseAmplitude,
	sizeRatio, enclosingRect, rotationCenter, this->m_countIndex,
	minMax.first, minMax.second,
	m_gridShown, m_clampingShown, m_gridCellCenterShown,
	onePixelInObjectSpace, m_glyphShown);

    // bind "tensor average" to texture unit 1
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetTensorAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.m_fbos->texture ());

    // bind "scalar average" (count) to texture unit 2
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.m_countFbos->texture ());
    
    // bind "overlay bar" to texture unit 0
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetOverlayBarTexUnit ()));    
    glBindTexture (GL_TEXTURE_1D, 
		   this->GetWidgetGl ().GetOverlayBarTexture (
		       this->GetViewNumber ()));

    widgetGl.ActivateViewShader (this->GetViewNumber (), enclose,
				 rotationCenter, angleDegrees);

    m_displayShaderProgram->release ();
    WarnOnOpenGLError ("TensorAverage::rotateAndDisplay");
}

template<typename Setter>
void TensorAverageTemplate<Setter>::calculateShaderParameters (
    G3D::Vector2 rotationCenter,

    G3D::Vector2* gridTranslation,
    float* gridCellLength,
    float* lineWidth, 
    float* sizeRatio,
    G3D::Rect2D* enclosingRect, 
    float* onePixelInObjectSpace) const
{
    const WidgetGl& widgetGl = this->GetWidgetGl ();
    ViewSettings& vs = widgetGl.GetViewSettings (this->GetViewNumber ());
    float scaleRatio = vs.GetScaleRatio ();
    float gridScaleRatio = vs.GetScaleRatio () * vs.GetGridScaleRatio ();

    *onePixelInObjectSpace = GetOnePixelInObjectSpace () * scaleRatio;
    *lineWidth = *onePixelInObjectSpace * CALL_MEMBER (vs, m_lineWidthRatio) ();

    *gridTranslation = vs.GetGridTranslation ().xy ();
    *gridCellLength = widgetGl.GetBubbleDiameter (this->GetViewNumber ()) * 
        gridScaleRatio;
    *enclosingRect = toRect2D (
	widgetGl.CalculateViewingVolume (
	    this->GetViewNumber (), 
	    ViewingVolumeOperation::ENCLOSE2D)) - rotationCenter;

    *sizeRatio = 
	CALL_MEMBER (widgetGl, m_sizeInitialRatio) (this->GetViewNumber ()) * 
	gridScaleRatio * CALL_MEMBER (vs, m_sizeRatio) ();
}

TensorAverage::TensorAverage (ViewNumber::Enum viewNumber, 
			      const WidgetGl& widgetGl,
			      FramebufferObjects& countFbos) :

    TensorAverageTemplate<SetterDeformation> (
	viewNumber, widgetGl, "tensor",
	&WidgetGl::GetDeformationSizeInitialRatio,
	&ViewSettings::GetDeformationSize,
	&ViewSettings::GetDeformationLineWidth,
	countFbos, 1)
{
}


// Template instantiations
//======================================================================
/// @cond
template class TensorAverageTemplate<SetterDeformation>;
template class TensorAverageTemplate<SetterVelocity>;
/// @endcond
