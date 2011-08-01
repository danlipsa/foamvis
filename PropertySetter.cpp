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
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "Utils.h"
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
    m_program->setAttributeValue (m_attributeLocation, value);
}

void SetterVertexAttribute::operator () ()
{
    // close to maxFloat. The same value is specified in GLSL and tested
    // against this value.
    GLfloat maxFloat = 3.40282e+38;
    m_program->setAttributeValue (m_attributeLocation, maxFloat);
}


// SetterDeformationTensor
// ======================================================================
void SetterDeformationTensor::operator () (const boost::shared_ptr<Body>& body)
{
    G3D::Matrix2 l = G3D::Matrix2::identity ();
    l[0][0] = body->GetDeformationEigenValue (0);
    l[1][1] = body->GetDeformationEigenValue (1);
    G3D::Matrix2 r;
    Matrix2SetColumn (&r, 0, body->GetDeformationEigenVector (0).xy ());
    Matrix2SetColumn (&r, 1, body->GetDeformationEigenVector (1).xy ());
    G3D::Matrix2 a = mult (mult (r, l), r.transpose ());
    // this function expects the matrix in row major order
    m_program->setAttributeValue (
	m_attributeLocation, a[0][0], a[0][1], a[1][0], a[1][1]);
}

void SetterDeformationTensor::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, 0, 0, 0, 0);
}
