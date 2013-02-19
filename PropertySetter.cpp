/**
 * @file PropertySetter.cpp
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Sends a property value to the graphics card
 */
#include "Body.h"
#include "ColorBarModel.h"
#include "Debug.h"
#include "Simulation.h"
#include "Settings.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "Utils.h"
#include "ViewSettings.h"

// WARNING: has to be the same as ScalarStore.frag and VectorStore.vert
const GLfloat MAX_FLOAT = 3.40282e+38;


// SetterTextureCoordinate
// ======================================================================

void SetterTextureCoordinate::operator () (
    const boost::shared_ptr<Body>& body)
{
    BodyScalar::Enum property = BodyScalar::FromSizeT (
	m_settings.GetViewSettings (m_viewNumber).GetBodyOrFaceScalar ());
    double value = body->GetScalarValue (property, Is2D ());
    double texCoord = 
	m_settings.GetViewSettings (m_viewNumber).
	GetColorBarModel ()->TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}

int SetterTextureCoordinate::GetBodyOrFaceScalar () const
{
    return m_settings.GetViewSettings (m_viewNumber).GetBodyOrFaceScalar ();
}

G3D::Matrix3 SetterTextureCoordinate::getRotation () const
{
    const ViewSettings& vs = m_settings.GetViewSettings (m_viewNumber);    
    G3D::Matrix4 modelRotation4; 
    G3D::glGetMatrix (GL_MODELVIEW_MATRIX, modelRotation4);
    return ToMatrix3 (modelRotation4 * (1.0 / vs.GetScaleRatio ()));
}


// SetterVertexAttribute
// ======================================================================
void SetterVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    BodyScalar::Enum bodyProperty = BodyScalar::FromSizeT (
	GetBodyOrFaceScalar ());
    double value = body->GetScalarValue (bodyProperty, Is2D ());
    m_program->setAttributeValue (m_attributeLocation, value);
}

void SetterVertexAttribute::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, MAX_FLOAT);
}


// SetterDeformation
// ======================================================================
void SetterDeformation::operator () (const boost::shared_ptr<Body>& body)
{
    G3D::Matrix2 a = ToMatrix2 (body->GetDeformationTensor (getRotation ()));

    // GLSL uses matrices in column order
    m_program->setAttributeValue (
	m_attributeLocation, a[0][0], a[1][0], a[0][1], a[1][1]);
    // debug
    //m_program->setAttributeValue (m_attributeLocation, 2., 1., 1., 2.);
}

void SetterDeformation::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, 0, 0, 0, 0);
}

int SetterDeformation::GetBodyOrFaceScalar () const
{
    return BodyScalar::DEFORMATION_EIGEN;
}


// SetterVelocity
// ======================================================================

void SetterVelocity::operator () (const boost::shared_ptr<Body>& body)
{
    G3D::Vector2 velocity = body->GetVelocity ().xy ();
    G3D::Matrix2 m = ToMatrix2 (getRotation ());
    velocity = m * velocity;
    m_program->setAttributeValue (m_attributeLocation, velocity.x, velocity.y);
}

void SetterVelocity::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, MAX_FLOAT, MAX_FLOAT);
}

int SetterVelocity::GetBodyOrFaceScalar () const
{
    return BodyScalar::VELOCITY_MAGNITUDE;
}
