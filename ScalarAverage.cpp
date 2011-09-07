/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageShaders.h"
#include "ScalarAverage.h"
#include "ShaderProgram.h"
#include "OpenGLUtils.h"
#include "Utils.h"
#include "GLWidget.h"

// Private classes/functions
// ======================================================================

// ScalarDisplay
// ======================================================================
/**
 * RGBA : sum, count, min, max
 */
class ScalarDisplay : public ShaderProgram
{
public:
    ScalarDisplay (const char* frag);
    void Bind (GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType);

    // assume the colorbar is alreay bound on texture unit 0
    GLint GetColorBarTexUnit ()
    {
	return 0;
    }
    GLint GetScalarAverageTexUnit ()
    {
	return 1;
    }

private:
    int m_displayTypeLocation;
    int m_minValueLocation;
    int m_maxValueLocation;
    int m_colorBarTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
};

ScalarDisplay::ScalarDisplay (const char* frag) :
    ShaderProgram (0, frag)
{
    m_displayTypeLocation = uniformLocation ("displayType");
    m_minValueLocation = uniformLocation("minValue");
    m_maxValueLocation = uniformLocation("maxValue");
    m_colorBarTexUnitLocation = uniformLocation("colorBarTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("scalarAverageTexUnit");
}

void ScalarDisplay::Bind (GLfloat minValue, GLfloat maxValue,
				 StatisticsType::Enum displayType)
{
    ShaderProgram::Bind ();
    setUniformValue (m_displayTypeLocation, displayType);
    setUniformValue (m_minValueLocation, minValue);
    setUniformValue (m_maxValueLocation, maxValue);
    setUniformValue (m_colorBarTexUnitLocation, GetColorBarTexUnit ());
    setUniformValue (m_scalarAverageTexUnitLocation, GetScalarAverageTexUnit ());
}



// ScalarAverage Methods
// ======================================================================

boost::shared_ptr<ScalarDisplay> ScalarAverage::m_displayShaderProgram;

void ScalarAverage::InitShaders ()
{
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("ScalarInit.frag")));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("ScalarStore.vert"), RESOURCE("ScalarStore.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
}


void ScalarAverage::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, TensorScalarFbo srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    m_displayShaderProgram->Bind (minValue, maxValue, displayType);
    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());
    GetGLWidget ().ActivateViewShader (viewNumber, enclose,
				       rotationCenter, angleDegrees);
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}
