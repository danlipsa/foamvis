/**
 * @file   ShaderProgram.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Interface for the ShaderProgram class
 */

#ifndef __SHADER_PROGRAM_H__
#define __SHADER_PROGRAM_H__

class ShaderProgram : public QGLShaderProgram
{
public:
    ShaderProgram (const char* vert, const char* frag);
    void Bind ();

private:
    boost::shared_ptr<QGLShader> m_fshader;
    boost::shared_ptr<QGLShader> m_vshader;
};




#endif //__SHADER_PROGRAM_H__

// Local Variables:
// mode: c++
// End:
