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
	float elipseSizeRatio,
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
    int m_ellipseSizeRatioLocation;
    int m_screenLowLocation;
    int m_screenHighLocation;
    int m_tensorAverageTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
};

TensorDisplay::TensorDisplay (const char* vert, const char* frag) :
    ShaderProgram (vert, frag)
{
    m_gridTranslationLocation = uniformLocation ("u_gridTranslation");
    m_cellLengthLocation = uniformLocation ("u_cellLength");
    m_lineWidthLocation = uniformLocation ("u_lineWidth");
    m_ellipseSizeRatioLocation = uniformLocation ("u_ellipseSizeRatio");
    m_screenLowLocation = uniformLocation ("u_screenLow");
    m_screenHighLocation = uniformLocation ("u_screenHigh");
    m_tensorAverageTexUnitLocation = uniformLocation("u_tensorAverageTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("u_scalarAverageTexUnit");
}

void TensorDisplay::Bind (
    G3D::Vector2 gridTranslation, float cellLength, float lineWidth,
    float ellipseSizeRatio,
    G3D::Vector2 screenLow, G3D::Vector2 screenHigh)
{
    ShaderProgram::Bind ();
    //cdbg << "ellipseSizeRatio: " << ellipseSizeRatio << endl;
    setUniformValue (
	m_gridTranslationLocation, gridTranslation[0], gridTranslation[1]);
    setUniformValue (m_cellLengthLocation, cellLength);
    setUniformValue (m_lineWidthLocation, lineWidth);
    setUniformValue (m_ellipseSizeRatioLocation, ellipseSizeRatio);
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


void TensorAverage::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    const G3D::Rect2D& destRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, FramebufferObjectPair srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)minValue;(void)maxValue;(void)displayType;
    G3D::Vector2 gridTranslation;float cellLength; float lineWidth;
    float ellipseSizeRatio;G3D::Rect2D screenRect;

    calculateShaderParameters (
	viewNumber, angleDegrees, &gridTranslation, &cellLength, &lineWidth,
	&ellipseSizeRatio, &screenRect);
    m_displayShaderProgram->Bind (
	gridTranslation, cellLength, lineWidth, 
	ellipseSizeRatio, screenRect.x0y0 (), screenRect.x1y1 ());

    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetTensorAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.first->texture ());

    // activate texture unit 2
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());

    GetGLWidget ().ActivateShader (
	viewNumber, destRect, enclose,
	rotationCenter, angleDegrees);
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
    WarnOnOpenGLError ("TensorAverage::rotateAndDisplay");
}

void TensorAverage::calculateShaderParameters (
    ViewNumber::Enum viewNumber, float angleDegrees,
    G3D::Vector2* gridTranslation, float* cellLength, float* lineWidth, 
    float* ellipseSizeRatio, G3D::Rect2D* screenRect) const
{
    const GLWidget& glWidget = GetGLWidget ();
    ViewSettings& vs = glWidget.GetViewSettings (viewNumber);
    float scaleRatio = vs.GetScaleRatio ();
    float gridScaleRatio = vs.GetScaleRatio () * vs.GetGridScaleRatio ();
    *gridTranslation = 
	(vs.GetGridTranslation () * scaleRatio + vs.GetTranslation ()).xy ();
    if (angleDegrees != 0.)
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
    *cellLength = glWidget.GetCellLength () * gridScaleRatio;
    *lineWidth = glWidget.GetOnePixelInObjectSpace () * 
	scaleRatio * glWidget.GetEllipseLineWidthRatio ();

    *ellipseSizeRatio = glWidget.GetEllipseSizeInitialRatio () * 
	glWidget.GetEllipseSizeRatio () * gridScaleRatio;
    G3D::AABox srcBox = glWidget.CalculateViewingVolume (viewNumber);
    *screenRect = G3D::Rect2D::xyxy (srcBox.low ().xy (), srcBox.high ().xy ());
}
