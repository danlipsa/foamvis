/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageShaders.h"
#include "TensorAverage.h"
#include "ShaderProgram.h"
#include "OpenGLUtils.h"

// Private classes/functions
// ======================================================================

// ScalarAverage Methods
// ======================================================================

void TensorAverage::InitShaders ()
{
    m_initShaderProgram.reset (new ShaderProgram (0, ":/TensorInit.frag"));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (":/TensorStore.vert", ":/TensorStore.frag"));
    m_addShaderProgram.reset (new AddShaderProgram (":/TensorAdd.frag"));
    m_removeShaderProgram.reset (new AddShaderProgram (":/TensorRemove.frag"));
}


void TensorAverage::display (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo)
{
}

void TensorAverage::displayAndRotate (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo,
    G3D::Vector2 rotationCenter, float angleDegrees)
{
}
