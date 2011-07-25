/**
 * @file   AverageShaders.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2010
 *
 * Implementation for the AverageShaders class 
 *
 */
#include "AverageShaders.h"

// StoreShaderProgram
// ======================================================================
StoreShaderProgram::StoreShaderProgram (const char* vert, const char* frag)
    : ShaderProgram (vert, frag)
{
    m_vValueIndex = attributeLocation("vValue");
}

// AddShaderProgram
// ======================================================================
AddShaderProgram::AddShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_previousTexUnitIndex = uniformLocation("previousTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void AddShaderProgram::Bind ()
{
    ShaderProgram::Bind ();
    setUniformValue (m_previousTexUnitIndex, GetPreviousTexUnit ());
    setUniformValue (m_stepTexUnitIndex, GetStepTexUnit ());
}
