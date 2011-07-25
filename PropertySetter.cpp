/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */
#include "Body.h"
#include "ColorBarModel.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "PropertySetter.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"

// SetterTextureCoordinate
// ======================================================================
void SetterTextureCoordinate::operator () (
    const boost::shared_ptr<Body>& body)
{
    BodyProperty::Enum property = 
	m_glWidget.GetViewSettings (m_viewNumber).GetBodyProperty ();
    double value = body->GetPropertyValue (property);
    double texCoord = 
	m_glWidget.GetViewSettings (m_viewNumber).
	GetColorBarModel ()->TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}

BodyProperty::Enum SetterTextureCoordinate::GetBodyProperty () const
{
    return m_glWidget.GetViewSettings (m_viewNumber).GetBodyProperty ();
}


// SetterVertexAttribute
// ======================================================================
void SetterVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (GetBodyProperty ());
    m_program->setAttributeValue (m_attributeIndex, value);
}

void SetterVertexAttribute::operator () ()
{
    // close to maxFloat. The same value is specified in GLSL and tested
    // against this value.
    GLfloat value = 3.40282e+38;
    m_program->setAttributeValue (m_attributeIndex, value);
}


// SetterDeformationTensor
// ======================================================================
void SetterDeformationTensor::operator () (const boost::shared_ptr<Body>& body)
{
    double value = body->GetPropertyValue (GetBodyProperty ());
    m_program->setAttributeValue (m_attributeIndex, value);
}

void SetterDeformationTensor::operator () ()
{
    // close to maxFloat. The same value is specified in GLSL and tested
    // against this value.
    GLfloat value = 3.40282e+38;
    m_program->setAttributeValue (m_attributeIndex, value);
}
