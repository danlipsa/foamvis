/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */

#include "ColorBarModel.h"
#include "GLWidget.h"
#include "PropertySetter.h"
#include "OpenGLUtils.h"

void TexCoordSetter::operator () (double value)
{
    double texCoord = m_glWidget.GetColorBarModel ().TexCoord (value);
    glTexCoord1f (texCoord); 
}

void TexCoordSetter::operator () ()
{
    glTexCoord1f (0); 
}


void VertexAttributeSetter::operator () ()
{
    double value = m_glWidget.GetColorBarModel ().GetInterval ().minValue ();
    m_program.setAttributeValue (m_attributeIndex, value);
}
