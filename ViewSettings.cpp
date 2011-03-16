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


// Static Fields
// ======================================================================

const size_t ViewSettings::NONE(numeric_limits<size_t>::max());

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
    m_selectedLight (LightNumber::LIGHT0),
    m_lightEnabled (0),
    m_lightPositionShown (0),
    m_angleOfView (0),
    m_bodyStationaryId (NONE),
    m_bodyStationaryTimeStep (0)
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

void ViewSettings::SetLightEnabled (LightNumber::Enum i, bool enabled)
{
    m_lightEnabled[i] = enabled;
    if (enabled)
	glEnable(GL_LIGHT0 + i);
    else
	glDisable (GL_LIGHT0 + i);
}



void ViewSettings::PositionLight (
    LightNumber::Enum lightNumber,
    const G3D::Vector3& initialLightPosition)
{
    if (IsLightEnabled (lightNumber))
    {
	G3D::Vector3 lp = initialLightPosition * 
	    GetLightNumberRatio (lightNumber);
	glPushMatrix ();
	glLoadMatrix (G3D::CoordinateFrame (
			  GetRotationLight (lightNumber)));
	if (IsDirectionalLightEnabled (lightNumber))
	{
	    glLightf(GL_LIGHT0 + lightNumber, GL_SPOT_CUTOFF, 180);
	    boost::array<GLfloat, 4> lightDirection = {{lp.x, lp.y, lp.z, 0}};
	    glLightfv(GL_LIGHT0 + lightNumber, GL_POSITION, &lightDirection[0]);
	}
	else
	{
	    glLightf(GL_LIGHT0 + lightNumber, GL_SPOT_CUTOFF, 15);
	    boost::array<GLfloat, 3> lightDirection = {{-lp.x, -lp.y, -lp.z}};
	    glLightfv(GL_LIGHT0 + lightNumber, 
		      GL_SPOT_DIRECTION, &lightDirection[0]);
	    glPushMatrix ();
	    glLoadIdentity ();
	    glTranslated (0, 0, - m_cameraDistance);
	    glMultMatrix (GetRotationLight (lightNumber));
	    GLfloat lightPosition[] = {lp.x, lp.y, lp.z, 1};
	    glLightfv(GL_LIGHT0 + lightNumber, GL_POSITION, lightPosition);
	    glPopMatrix ();
	}
	glPopMatrix ();
    }
}

void ViewSettings::CalculateCameraDistance (
    const G3D::AABox& centeredViewingVolume)
{
    G3D::Vector3 diagonal =
	centeredViewingVolume.high () - centeredViewingVolume.low ();
    if (GetAngleOfView () == 0)
	m_cameraDistance = diagonal.z;
    else
    {
	// distance from the camera to the middle of the bounding box
	m_cameraDistance = diagonal.y / 2 /
	    tan (GetAngleOfView () * M_PI / 360) + diagonal.z / 2;
    }
}

void ViewSettings::SetLightingParameters (
    const G3D::Vector3& initialLightPosition)
{
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
    {
	LightNumber::Enum lightNumber = LightNumber::Enum (i);
	if (IsLightEnabled (lightNumber))
	{
	    SetLightEnabled (lightNumber, true);
	    PositionLight (lightNumber, initialLightPosition);
	}
    }
    EnableLighting ();
}

bool ViewSettings::IsBodyContext (size_t bodyId) const
{
    return m_bodyContext.find (bodyId) != m_bodyContext.end ();
}
