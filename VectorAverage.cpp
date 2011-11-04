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
#include "VectorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"


// Private classes/functions
// ======================================================================

// VectorDisplay
// ======================================================================
/**
 * RGBA : sum, count, min, max
 */
class VectorDisplay : public ShaderProgram
{
public:
    VectorDisplay (const char* vert, const char* frag);
    void Bind (
	G3D::Vector2 gridTranslation, float cellLength, float lineWidth, 
	float elipseSizeRatio, G3D::Rect2D enclosingRect,
	G3D::Vector2 rotationCenter, bool gridShown);

    GLint GetVectorAverageTexUnit ()
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
};

VectorDisplay::VectorDisplay (const char* vert, const char* frag) :
    ShaderProgram (vert, frag)
{
    m_gridTranslationLocation = uniformLocation ("u_gridTranslationE");
    m_cellLengthLocation = uniformLocation ("u_cellLength");
    m_lineWidthLocation = uniformLocation ("u_lineWidth");
    m_ellipseSizeRatioLocation = uniformLocation ("u_ellipseSizeRatio");
    m_enclosingRectLowLocation = uniformLocation ("u_enclosingRect.m_low");
    m_enclosingRectHighLocation = uniformLocation ("u_enclosingRect.m_high");
    m_rotationCenterLocation = uniformLocation ("u_rotationCenter");
    m_tensorAverageTexUnitLocation = uniformLocation("u_tensorAverageTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("u_scalarAverageTexUnit");
    m_gridShownLocation = uniformLocation ("u_gridShown");
}

void VectorDisplay::Bind (
    G3D::Vector2 gridTranslation, float cellLength, float lineWidth,
    float ellipseSizeRatio, G3D::Rect2D enclosingRect, 
    G3D::Vector2 rotationCenter, bool gridShown)
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
	m_tensorAverageTexUnitLocation, GetVectorAverageTexUnit ());
    setUniformValue (
	m_scalarAverageTexUnitLocation, GetScalarAverageTexUnit ());
    setUniformValue (m_gridShownLocation, gridShown);
}


// VectorAverage Methods
// ======================================================================

boost::shared_ptr<VectorDisplay> VectorAverage::m_displayShaderProgram;

void VectorAverage::InitShaders ()
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
	new VectorDisplay (RESOURCE("TensorDisplay.vert"),
			   RESOURCE("TensorDisplay.frag")));
}


void VectorAverage::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, TensorScalarFbo srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)minValue;(void)maxValue;(void)displayType;
    G3D::Vector2 gridTranslation;float cellLength; float lineWidth;
    float ellipseSizeRatio;G3D::Rect2D enclosingRect;

    calculateShaderParameters (
	viewNumber, rotationCenter, angleDegrees, &gridTranslation, &cellLength, 
	&lineWidth, &ellipseSizeRatio, &enclosingRect);
    m_displayShaderProgram->Bind (
	gridTranslation, cellLength, lineWidth, 
	ellipseSizeRatio, enclosingRect, rotationCenter,
	m_gridShown);

    // activate texture unit 1 - tensor average
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetVectorAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.first->texture ());

    // activate texture unit 2 - scalar average
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());
    
    GetGLWidget ().ActivateViewShader (viewNumber, enclose,
				       rotationCenter, angleDegrees);
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
    WarnOnOpenGLError ("VectorAverage::rotateAndDisplay");
}

void VectorAverage::calculateShaderParameters (
    ViewNumber::Enum viewNumber, G3D::Vector2 rotationCenter, float angleDegrees,
    G3D::Vector2* gridTranslation, float* cellLength, float* lineWidth, 
    float* ellipseSizeRatio, G3D::Rect2D* enclosingRect) const
{
    const GLWidget& glWidget = GetGLWidget ();
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
	scaleRatio * glWidget.GetDeformationEllipseLineWidthRatio ();
    *ellipseSizeRatio = glWidget.GetDeformationEllipseSizeInitialRatio (
	viewNumber) * 
	glWidget.GetDeformationEllipseSizeRatio () * gridScaleRatio;
    *enclosingRect = 
	glWidget.CalculateViewEnclosingRect (viewNumber) - rotationCenter;
}
