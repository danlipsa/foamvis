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
    BodyProperty::Enum property = m_glWidget.GetBodyProperty (m_view);
    double value = body->GetPropertyValue (property);
    double texCoord = m_glWidget.GetColorBarModel (m_view).TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterValueTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}

BodyProperty::Enum SetterValueTextureCoordinate::GetBodyProperty () const
{
    return m_glWidget.GetBodyProperty (m_view);
}

void SetterValueVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (GetBodyProperty ());
    m_program->setAttributeValue (m_attributeIndex, value);
}

void SetterValueVertexAttribute::operator () ()
{
    double value = m_glWidget.GetColorBarModel (
	GetView ()).GetInterval ().minValue ();
    m_program->setAttributeValue (m_attributeIndex, value);
}


