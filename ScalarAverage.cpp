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

// Private classes/functions
// ======================================================================

// DisplayShaderProgram
// ======================================================================
/**
 * RGBA : sum, count, min, max
 */
class DisplayShaderProgram : public ShaderProgram
{
public:
    DisplayShaderProgram (const char* frag);
    void Bind (GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType);

    // assume the colorbar is alreay bound on texture unit 0
    GLint GetColorBarTexUnit ()
    {
	return 0;
    }
    GLint GetResultTexUnit ()
    {
	return 1;
    }

private:
    int m_displayTypeIndex;
    int m_minValueIndex;
    int m_maxValueIndex;
    int m_colorBarTexUnitIndex;
    int m_resultTexUnitIndex;
};

DisplayShaderProgram::DisplayShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_displayTypeIndex = uniformLocation ("displayType");
    m_minValueIndex = uniformLocation("minValue");
    m_maxValueIndex = uniformLocation("maxValue");
    m_colorBarTexUnitIndex = uniformLocation("colorBarTexUnit");
    m_resultTexUnitIndex = uniformLocation("resultTexUnit");
}

void DisplayShaderProgram::Bind (GLfloat minValue, GLfloat maxValue,
				 StatisticsType::Enum displayType)
{
    ShaderProgram::Bind ();
    setUniformValue (m_displayTypeIndex, displayType);
    setUniformValue (m_minValueIndex, minValue);
    setUniformValue (m_maxValueIndex, maxValue);
    setUniformValue (m_colorBarTexUnitIndex, GetColorBarTexUnit ());
    setUniformValue (m_resultTexUnitIndex, GetResultTexUnit ());
}



// ScalarAverage Methods
// ======================================================================

boost::shared_ptr<DisplayShaderProgram> ScalarAverage::m_displayShaderProgram;

void ScalarAverage::InitShaders ()
{
    m_initShaderProgram.reset (new ShaderProgram (0, ":/ScalarInit.frag"));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (":/ScalarStore.vert", ":/ScalarStore.frag"));
    m_addShaderProgram.reset (new AddShaderProgram (":/ScalarAdd.frag"));
    m_removeShaderProgram.reset (new AddShaderProgram (":/ScalarRemove.frag"));
    m_displayShaderProgram.reset (
	new DisplayShaderProgram (":/ScalarDisplay.frag"));
}


void ScalarAverage::display (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo)
{
    m_displayShaderProgram->Bind (minValue, maxValue, displayType);
    glActiveTexture (TextureEnum (m_displayShaderProgram->GetResultTexUnit ()));
    RenderFromFbo (viewRect, srcFbo);
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}

void ScalarAverage::displayAndRotate (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo,
    G3D::Vector2 rotationCenter, float angleDegrees)
{
    m_displayShaderProgram->Bind (minValue, maxValue, displayType);
    glActiveTexture (TextureEnum (m_displayShaderProgram->GetResultTexUnit ()));
    RenderFromFboAndRotate (viewRect, srcFbo, 
			    rotationCenter, angleDegrees);
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}
