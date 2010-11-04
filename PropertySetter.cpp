/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */

#include "PropertySetter.h"
#include "GLWidget.h"

void TexCoordSetter::operator () (double value)
{
    double texCoord = m_glWidget.TexCoord (value);
    glTexCoord1f (texCoord); 
}
