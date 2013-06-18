/**
 * @file   AverageShaders.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * @brief Shaders used for computing a pixel-based time-average of attributes.
 */

#ifndef __AVERAGE_SHADERS_H__
#define __AVERAGE_SHADERS_H__

#include "ShaderProgram.h"

/**
 * @brief Shader used to store attribute values into the graphics card.
 */
class StoreShaderProgram : public ShaderProgram
{
public:
    StoreShaderProgram (const char* vert, const char* frag);
    int GetVValueLocation () const
    {
	return m_vValueLocation;
    }
private:
    int m_vValueLocation;
};


/**
 * @brief Shader used for the summation step in computing the average.
 */
class AddShaderProgram : public ShaderProgram
{
public:
    AddShaderProgram (const char* frag);
    void Bind ();
    GLint GetPreviousTexUnit ()
    {
	return 1;
    }
    GLint GetStepTexUnit ()
    {
	return 2;
    }
protected:
    int m_previousTexUnitLocation;
    int m_stepTexUnitLocation;
};


#endif //__AVERAGE_SHADERS_H__

// Local Variables:
// mode: c++
// End:
