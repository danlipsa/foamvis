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

// @todo Implement clamping for vector size: Initially, render the
// vector so that the maximum is fully rendered. Then, when
// increasing the size, keep the maximum size fixed.


VectorAverage::VectorAverage (const GLWidget& glWidget, 
			      FramebufferObjects& scalarAverageFbos) :
    TensorAverageTemplate<SetterVelocity> (
	glWidget,
	&GLWidget::GetVelocitySizeInitialRatio,
	&ViewSettings::GetVelocitySize,
	&ViewSettings::GetVelocityLineWidth,
	scalarAverageFbos)
{
}

void VectorAverage::InitShaders ()
{
    cdbg << "==== VectorAverage ====" << endl;
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("TensorInit.frag")));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("VectorStore.vert"), RESOURCE("TensorStore.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorRemove.frag")));
    m_displayShaderProgram.reset (
	new TensorDisplay (RESOURCE("TensorDisplay.vert"),
			   RESOURCE("VectorDisplay.frag")));
}
