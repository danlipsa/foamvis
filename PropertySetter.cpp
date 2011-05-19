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
#include "ViewSettings.h"

// SetterValueTextureCoordinate
// ======================================================================
void SetterValueTextureCoordinate::operator () (
    const boost::shared_ptr<Body>& body)
{
    BodyProperty::Enum property = 
	m_glWidget.GetViewSettings (m_viewNumber)->GetBodyProperty ();
    double value = body->GetPropertyValue (property);
    double texCoord = 
	m_glWidget.GetViewSettings (m_viewNumber)->
	GetColorBarModel ()->TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterValueTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}

BodyProperty::Enum SetterValueTextureCoordinate::GetBodyProperty () const
{
    return m_glWidget.GetViewSettings (m_viewNumber)->GetBodyProperty ();
}


// SetterValueVertexAttribute
// ======================================================================
void SetterValueVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (GetBodyProperty ());
    m_program->setAttributeValue (m_attributeIndex, value);
}

void SetterValueVertexAttribute::operator () ()
{
    double value = m_glWidget.GetViewSettings (
	m_viewNumber)->GetColorBarModel ()->GetInterval ().minValue ();
    m_program->setAttributeValue (m_attributeIndex, value);
}
