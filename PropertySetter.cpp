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
    G3D::Matrix3 modelRotation3 = 
	m_glWidget.GetViewSettings (m_viewNumber).GetRotationModel ();
    G3D::Matrix2 modelRotation;
    Matrix2SetColumn (&modelRotation, 0, modelRotation3.column (0).xy ());
    Matrix2SetColumn (&modelRotation, 1, modelRotation3.column (1).xy ());
    cdbg << modelRotation << endl;
    G3D::Vector2 first = body->GetDeformationEigenVector (0).xy ();
    G3D::Vector2 second = body->GetDeformationEigenVector (1).xy ();
    Matrix2SetColumn (&r, 0, first);
    Matrix2SetColumn (&r, 1, second);
    G3D::Matrix2 a = mult (modelRotation, mult (mult (r, l), r.transpose ()));
    // GLSL uses matrices in column order
    m_program->setAttributeValue (
	m_attributeLocation, a[0][0], a[1][0], a[0][1], a[1][1]);
    /*
    cdbg << "r*l=" << mult (r, l) << endl;
    cdbg << "Eigen val.: " << l[0][0] << ", " << l[1][1] << endl;
    cdbg << "Eigen vec.: " << first << ", " << second << endl;
    cdbg << a << endl << endl;
    */
    // debug
    //m_program->setAttributeValue (m_attributeLocation, 2., 1., 1., 2.);
}

void SetterDeformationTensor::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, 0, 0, 0, 0);
}
