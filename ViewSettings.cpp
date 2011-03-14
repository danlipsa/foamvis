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

ViewSettings::ViewSettings (const GLWidget& glWidget) :
    m_viewType (ViewType::COUNT),
    m_bodyProperty (BodyProperty::NONE),
    m_statisticsType (StatisticsType::AVERAGE),
    m_listCenterPaths (0),
    m_colorBarTexture (0),
    m_displayFaceStatistics (new DisplayFaceStatistics (glWidget)),
    m_rotationModel (G3D::Matrix3::identity ()),
    m_scaleRatio (1),
    m_translation (G3D::Vector3::zero ())
{
    initTexture ();
    initList ();
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
