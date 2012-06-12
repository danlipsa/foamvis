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
#include "DebugStream.h"
#include "Simulation.h"
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
    BodyProperty::Enum property = BodyProperty::FromSizeT (
	m_widgetGl.GetViewSettings (m_viewNumber).GetBodyOrFaceProperty ());
    double value = body->GetPropertyValue (property);
    double texCoord = 
	m_widgetGl.GetViewSettings (m_viewNumber).
	GetColorBarModel ()->TexCoord (value);
    glTexCoord1f (texCoord); 
}

void SetterTextureCoordinate::operator () ()
{
    glTexCoord1f (0); 
}

int SetterTextureCoordinate::GetBodyOrFaceProperty () const
{
    return m_widgetGl.GetViewSettings (m_viewNumber).GetBodyOrFaceProperty ();
}

G3D::Matrix2 SetterTextureCoordinate::getRotation () const
{
    const ViewSettings& vs = m_widgetGl.GetViewSettings (m_viewNumber);    
    G3D::Matrix4 modelRotation4; 
    G3D::glGetMatrix (GL_MODELVIEW_MATRIX, modelRotation4);
    return ToMatrix2 (modelRotation4) / vs.GetScaleRatio ();
}


// SetterVertexAttribute
// ======================================================================
void SetterVertexAttribute::operator () (
    const boost::shared_ptr<Body>& body)
{
    BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT (
	GetBodyOrFaceProperty ());
    double value = body->GetPropertyValue (bodyProperty);
    m_program->setAttributeValue (m_attributeLocation, value);
}

void SetterVertexAttribute::operator () ()
{
    // max_float
    // WARNING: has to be the same as ScalarStore.frag
    GLfloat maxFloat = 3.40282e+38;
    m_program->setAttributeValue (m_attributeLocation, maxFloat);
}


// SetterDeformation
// ======================================================================
void SetterDeformation::operator () (const boost::shared_ptr<Body>& body)
{
    // Practical Linear Algebra, A Geometry Toolbox, 
    // Gerald Farin, Dianne Hansford, Sec 7.5
    G3D::Matrix2 l = G3D::Matrix2::identity ();
    l[0][0] = body->GetDeformationEigenValue (0);
    l[1][1] = body->GetDeformationEigenValue (1);
    G3D::Matrix2 modelRotation = getRotation ();
    G3D::Matrix2 r = 
	mult (modelRotation,
	      MatrixFromColumns (body->GetDeformationEigenVector (0).xy (),
				 body->GetDeformationEigenVector (1).xy ()));
    G3D::Matrix2 a = mult (mult (r, l), r.transpose ());

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

int SetterDeformation::GetBodyOrFaceProperty () const
{
    return BodyProperty::DEFORMATION_EIGEN;
}


// SetterVelocity
// ======================================================================

void SetterVelocity::operator () (const boost::shared_ptr<Body>& body)
{
    G3D::Vector2 velocity = body->GetVelocity ().xy ();
    G3D::Matrix2 m = getRotation ();
    velocity = m * velocity;
    m_program->setAttributeValue (m_attributeLocation, velocity.x, velocity.y);
}

void SetterVelocity::operator () ()
{
    m_program->setAttributeValue (m_attributeLocation, 0, 0);
}

int SetterVelocity::GetBodyOrFaceProperty () const
{
    return BodyProperty::VELOCITY_MAGNITUDE;
}
