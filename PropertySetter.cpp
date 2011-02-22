/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */
#include "Body.h"
#include "ColorBarModel.h"
#include "GLWidget.h"
#include "PropertySetter.h"
#include "OpenGLUtils.h"

void SetterValueTextureCoordinate::operator () (
    const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (m_property);
    double texCoord = m_glWidget.GetColorBarModel ().TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterValueTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}


void SetterValueVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (m_property);
    m_program->setAttributeValue (m_attributeIndex, value);
}

void SetterValueVertexAttribute::operator () ()
{
    double value = m_glWidget.GetColorBarModel ().GetInterval ().minValue ();
    m_program->setAttributeValue (m_attributeIndex, value);
}


