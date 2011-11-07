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
#include "ScalarAverage.h"
#include "ScalarDisplay.h"
#include "ShaderProgram.h"
#include "Utils.h"


// Private classes/functions
// ======================================================================


// ScalarAverage Methods
// ======================================================================

template<typename PropertySetter>
boost::shared_ptr<ScalarDisplay> 
ScalarAverageTemplate<PropertySetter>::m_displayShaderProgram;

template<typename PropertySetter>
void ScalarAverageTemplate<PropertySetter>::InitShaders ()
{
    cdbg << "==== ScalarAverageTemplate ====" << endl;
    ImageBasedAverage<PropertySetter>::m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("ScalarInit.frag")));
    ImageBasedAverage<PropertySetter>::m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("ScalarStore.vert"), RESOURCE("ScalarStore.frag")));
    ImageBasedAverage<PropertySetter>::m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    ImageBasedAverage<PropertySetter>::m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    ScalarAverageTemplate<PropertySetter>::m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
}

template<typename PropertySetter>
void ScalarAverageTemplate<PropertySetter>::rotateAndDisplay (
    ViewNumber::Enum viewNumber,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, 
    typename ImageBasedAverage<PropertySetter>::TensorScalarFbo srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    m_displayShaderProgram->Bind (minValue, maxValue, displayType);
    // activate texture unit 1 - scalar average
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());
    this->GetGLWidget ().ActivateViewShader (viewNumber, enclose,
					     rotationCenter, angleDegrees);
    // activate texture unit 0
    this->glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}


// Template instantiations
//======================================================================
/// @cond
template class ScalarAverageTemplate<SetterVertexAttribute>;
template class ScalarAverageTemplate<SetterNop>;
/// @endcond
