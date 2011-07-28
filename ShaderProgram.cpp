/**
 * @file   SharedProgram.cpp
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ShaderProgram class 
 *
 */

#include "Debug.h"
#include "ShaderProgram.h"
#include "Utils.h"

ShaderProgram::ShaderProgram (const char* vert, const char* frag)
{
    if (vert != 0)
    {
	m_vshader = CreateShader (vert, QGLShader::Vertex);
	addShader(m_vshader.get ());
    }
    m_fshader = CreateShader (frag, QGLShader::Fragment);
    addShader(m_fshader.get ());
    link();
}

void ShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for ShaderProgram");
}
