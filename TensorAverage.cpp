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
#include "ShaderProgram.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"


// Private classes/functions
// ======================================================================

// TensorDisplay
// ======================================================================
/**
 * RGBA : sum, count, min, max
 */
class TensorDisplay : public ShaderProgram
{
public:
    TensorDisplay (const char* vert, const char* frag);
    void Bind (
	G3D::Vector2 gridTranslation, float cellLength, float lineWidth,
	G3D::Vector2 screenLow, G3D::Vector2 screenHigh);

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
    int m_screenLowLocation;
    int m_screenHighLocation;
    int m_tensorAverageTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
};

TensorDisplay::TensorDisplay (const char* vert, const char* frag) :
    ShaderProgram (vert, frag)
{
    m_gridTranslationLocation = uniformLocation ("gridTranslation");
    m_cellLengthLocation = uniformLocation ("cellLength");
    m_lineWidthLocation = uniformLocation ("lineWidth");
    m_screenLowLocation = uniformLocation ("screenLow");
    m_screenHighLocation = uniformLocation ("screenHigh");
    m_tensorAverageTexUnitLocation = uniformLocation("tensorAverageTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("scalarAverageTexUnit");
}

void TensorDisplay::Bind (G3D::Vector2 gridTranslation, 
			  float cellLength, float lineWidth,
			  G3D::Vector2 screenLow, G3D::Vector2 screenHigh)
{
    ShaderProgram::Bind ();
    setUniformValue (
	m_gridTranslationLocation, gridTranslation[0], gridTranslation[1]);
    setUniformValue (m_cellLengthLocation, cellLength);
    setUniformValue (m_lineWidthLocation, lineWidth);
    setUniformValue (m_screenLowLocation, screenLow[0], screenLow[1]);
    setUniformValue (m_screenHighLocation, screenHigh[0], screenHigh[1]);
    setUniformValue (
	m_tensorAverageTexUnitLocation, GetTensorAverageTexUnit ());
    setUniformValue (
	m_scalarAverageTexUnitLocation, GetScalarAverageTexUnit ());
}


// ScalarAverage Methods
// ======================================================================

boost::shared_ptr<TensorDisplay> TensorAverage::m_displayShaderProgram;

void TensorAverage::InitShaders ()
{
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("TensorInit.frag")));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("TensorStore.vert"), RESOURCE("TensorStore.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorRemove.frag")));
    m_displayShaderProgram.reset (
	new TensorDisplay (RESOURCE("TensorDisplay.vert"),
			   RESOURCE("TensorDisplay.frag")));
}

void TensorAverage::InitStep (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<QGLFramebufferObject> scalarAverage)
{
    m_scalarAverage = scalarAverage;
    Average::InitStep (viewNumber);
}


void TensorAverage::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)minValue;
    (void)maxValue;
    (void)displayType;

    const GLWidget& glWidget = GetGLWidget ();
    ViewSettings& vs = glWidget.GetViewSettings (viewNumber);
    G3D::Vector2 gridTranslation = 
	(vs.GetGridTranslation () * vs.GetScaleRatio () * 
	 vs.GetGridScaleRatio () + vs.GetTranslation ()).xy ();
    double cellLength = 
	glWidget.GetCellLength ().first * vs.GetScaleRatio () * 
	vs.GetGridScaleRatio ();
    // @todo why do I have to use the scale ratio?
    double lineWidth = glWidget.GetOnePixelInObjectSpace () * 
	glWidget.GetEllipseLineWidthRatio ();
    G3D::AABox srcBox = glWidget.CalculateViewingVolume (viewNumber);
    m_displayShaderProgram->Bind (gridTranslation, cellLength, lineWidth,
				  srcBox.low ().xy (), srcBox.high ().xy ());

    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetTensorAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.texture ());

    // activate texture unit 2
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_scalarAverage->texture ());

    GetGLWidget ().ActivateShader (
	viewNumber, viewRect, rotationCenter, angleDegrees);
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
    WarnOnOpenGLError ("TensorAverage::rotateAndDisplay");
}
