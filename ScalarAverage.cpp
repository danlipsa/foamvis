/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageShaders.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "ScalarAverage.h"
#include "ScalarDisplay.h"
#include "ShaderProgram.h"
#include "Utils.h"


// Private classes/functions
// ======================================================================


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
