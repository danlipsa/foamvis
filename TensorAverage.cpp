/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageShaders.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"


// Private classes/functions
// ======================================================================

TensorDisplay::TensorDisplay (const char* vert, const char* frag) :
    ShaderProgram (vert, frag)
{
    m_gridTranslationLocation = uniformLocation ("u_gridTranslationE");
    m_cellLengthLocation = uniformLocation ("u_cellLength");
    m_lineWidthLocation = uniformLocation ("u_lineWidth");
    m_ellipseSizeRatioLocation = uniformLocation ("u_sizeRatio");
    m_enclosingRectLowLocation = uniformLocation ("u_enclosingRect.m_low");
    m_enclosingRectHighLocation = uniformLocation ("u_enclosingRect.m_high");
    m_rotationCenterLocation = uniformLocation ("u_rotationCenter");
    m_tensorAverageTexUnitLocation = uniformLocation("u_tensorAverageTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("u_scalarAverageTexUnit");
    m_gridShownLocation = uniformLocation ("u_gridShown");
    m_gridCellCenterShownLocation = 
	uniformLocation ("u_gridCellCenterShown");
}

void TensorDisplay::Bind (
    G3D::Vector2 gridTranslation, float cellLength, float lineWidth,
    float ellipseSizeRatio, G3D::Rect2D enclosingRect, 
    G3D::Vector2 rotationCenter,
    bool gridShown, bool gridCellCenterShown)
{
    ShaderProgram::Bind ();
    setUniformValue (
	m_gridTranslationLocation, gridTranslation[0], gridTranslation[1]);
    setUniformValue (m_cellLengthLocation, cellLength);
    setUniformValue (m_lineWidthLocation, lineWidth);
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
    setUniformValue (m_gridShownLocation, gridShown);
    setUniformValue (m_gridCellCenterShownLocation, 
		     gridCellCenterShown);
}


// TensorAverage Methods
// ======================================================================

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
void TensorAverageTemplate<Setter>::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, 
    typename ImageBasedAverage<Setter>::TensorScalarFbo srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)minValue;(void)maxValue;(void)displayType;
    G3D::Vector2 gridTranslation;float cellLength; float lineWidth;
    float sizeRatio;G3D::Rect2D enclosingRect;

    calculateShaderParameters (
	viewNumber, rotationCenter, angleDegrees, &gridTranslation, 
	&cellLength, &lineWidth, &sizeRatio, &enclosingRect);
    m_displayShaderProgram->Bind (
	gridTranslation, cellLength, lineWidth, 
	sizeRatio, enclosingRect, rotationCenter,
	m_gridShown, m_gridCellCenterShown);

    // activate texture unit 1 - tensor average
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetTensorAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.first->texture ());

    // activate texture unit 2 - scalar average
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());
    
    this->GetGLWidget ().ActivateViewShader (viewNumber, enclose,
				       rotationCenter, angleDegrees);
    // activate texture unit 0
    this->glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
    WarnOnOpenGLError ("TensorAverage::rotateAndDisplay");
}

template<typename Setter>
void TensorAverageTemplate<Setter>::calculateShaderParameters (
    ViewNumber::Enum viewNumber, G3D::Vector2 rotationCenter, 
    float angleDegrees,
    G3D::Vector2* gridTranslation, float* cellLength, float* lineWidth, 
    float* sizeRatio, G3D::Rect2D* enclosingRect) const
{
    const GLWidget& glWidget = this->GetGLWidget ();
    ViewSettings& vs = glWidget.GetViewSettings (viewNumber);
    float scaleRatio = vs.GetScaleRatio ();
    float gridScaleRatio = vs.GetScaleRatio () * vs.GetGridScaleRatio ();
    *gridTranslation = (vs.GetGridTranslation () * scaleRatio).xy ();
    if (false /*angleDegrees != 0.*/)
    {
	//cdbg << angleDegrees << endl;
	//cdbg << "gridTranslation: " << gridTranslation << endl;
	float cosa = cos (angleDegrees);
	float sina = sin (angleDegrees);
	G3D::Matrix2 m;
	Matrix2SetColumn (&m, 0, G3D::Vector2 (cosa, sina));
	Matrix2SetColumn (&m, 1, G3D::Vector2 (-sina, cosa));
	*gridTranslation = m * (*gridTranslation);
	//cdbg << "gridTranslation: " << gridTranslation << endl << endl;
    }
    *cellLength = glWidget.GetCellLength (viewNumber) * gridScaleRatio;
    *lineWidth = glWidget.GetOnePixelInObjectSpace () * 
	scaleRatio * CALL_MEMBER_FN (vs, m_lineWidthRatio) ();
    *sizeRatio = 
	CALL_MEMBER_FN (glWidget, m_sizeInitialRatio) (viewNumber) * 
	CALL_MEMBER_FN (vs, m_sizeRatio) () * gridScaleRatio;
    *enclosingRect = 
	glWidget.CalculateViewEnclosingRect (viewNumber) - rotationCenter;
}

TensorAverage::TensorAverage (const GLWidget& glWidget,
			      FramebufferObjects& scalarAverageFbos) :
    TensorAverageTemplate<SetterDeformation> (
	glWidget, &GLWidget::GetDeformationSizeInitialRatio,
	&ViewSettings::GetDeformationSize,
	&ViewSettings::GetDeformationLineWidth,
	scalarAverageFbos)
{
}


// Template instantiations
//======================================================================
/// @cond
template class TensorAverageTemplate<SetterDeformation>;
template class TensorAverageTemplate<SetterVelocity>;
/// @endcond
