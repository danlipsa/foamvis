/**
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 * @date 10 March 2011
 *
 * Definitions for the view settings
 */

#include "DebugStream.h"
#include "DisplayFaceStatistics.h"
#include "ViewSettings.h"

// Private Functions
// ======================================================================

void initialize (boost::array<GLfloat, 4>& colors,
		 const boost::array<GLfloat, 4>& values)
{
    copy (values.begin (), values.end (), colors.begin ());
}

// Methods
// ======================================================================

ViewSettings::ViewSettings (const GLWidget& glWidget) :
    m_viewType (ViewType::COUNT),
    m_bodyProperty (BodyProperty::NONE),
    m_statisticsType (StatisticsType::AVERAGE),
    m_listCenterPaths (0),
    m_colorBarTexture (0),
    m_displayFaceStatistics (new DisplayFaceStatistics (glWidget)),
    m_rotationModel (G3D::Matrix3::identity ()),
    m_scaleRatio (1),
    m_translation (G3D::Vector3::zero ()),
    m_lightingEnabled (false),
    m_selectedLight (LightNumber::TOP_RIGHT),
    m_lightEnabled (0),
    m_lightPositionShown (0)
{
    initTexture ();
    initList ();
    setInitialLightParameters ();
}

ViewSettings::~ViewSettings ()
{
    glDeleteTextures (1, &m_colorBarTexture);
    glDeleteLists (m_listCenterPaths, 1);
}

void ViewSettings::initTexture ()
{
    glGenTextures (1, &m_colorBarTexture);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ViewSettings::initList ()
{
    m_listCenterPaths = glGenLists (1);
}

void ViewSettings::setInitialLightParameters ()
{
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
    {
	SetInitialLightPosition (LightNumber::Enum(i));
	m_directionalLightEnabled[i] = true;
    }

    boost::array<
    boost::array<GLfloat,4>, LightType::COUNT> light = {{
	// default (0, 0, 0, 1)
	{{0, 0, 0, 1.0}},
	// default (1, 1, 1, 1)
	{{1.0, 1.0, 1.0, 1.0}},
	// default (1, 1, 1, 1)
	{{1.0, 1.0, 1.0, 1.0}}
	}};
    for(size_t i = 0; i < LightType::COUNT; ++i)
	for_each (m_light[i].begin (), m_light[i].end (),
		  boost::bind (initialize, _1, light[i]));
}

void ViewSettings::SetInitialLightPosition (LightNumber::Enum i)
{
    m_lightPositionRatio[i] = 1;
    m_rotationLight[i] = G3D::Matrix3::identity ();
}

void ViewSettings::EnableLighting ()
{
    m_lightingEnabled = m_lightEnabled.any ();
    if (m_lightingEnabled)
	glEnable (GL_LIGHTING);
    else
	glDisable (GL_LIGHTING);
}

void ViewSettings::PositionLight (
    LightNumber::Enum i,
    const G3D::Vector3& initialLightPosition,
    double cameraDistance)
{
    if (IsLightEnabled (i))
    {
	G3D::Vector3 lp = initialLightPosition * GetLightNumberRatio (i);
	glPushMatrix ();
	glLoadMatrix (G3D::CoordinateFrame (GetRotationLight (i)));
	if (IsDirectionalLightEnabled (i))
	{
	    glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 180);
	    boost::array<GLfloat, 4> lightDirection = {{lp.x, lp.y, lp.z, 0}};
	    glLightfv(GL_LIGHT0 + i, GL_POSITION, &lightDirection[0]);
	}
	else
	{
	    glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 15);
	    boost::array<GLfloat, 3> lightDirection = {{-lp.x, -lp.y, -lp.z}};
	    glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, &lightDirection[0]);
	    glPushMatrix ();
	    glLoadIdentity ();
	    glTranslated (0, 0, - cameraDistance);
	    glMultMatrix (GetRotationLight (i));
	    GLfloat lightPosition[] = {lp.x, lp.y, lp.z, 1};
	    glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPosition);
	    glPopMatrix ();
	}
	glPopMatrix ();
    }
}
