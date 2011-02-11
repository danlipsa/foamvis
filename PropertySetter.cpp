/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */


#include "GLWidget.h"
#include "PropertySetter.h"
#include "OpenGLUtils.h"

void TexCoordSetter::operator () (double value)
{
    glColor (Qt::white);
    double texCoord = m_glWidget.TexCoord (value);
    glTexCoord1f (texCoord); 
}
