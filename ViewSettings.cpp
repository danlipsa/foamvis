/**
 * @file   ViewSettings.cpp
 * @author Dan R. Lipsa
 * @date 10 March 2011
 *
 * Definitions for the view settings
 */

#include "Body.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "DebugStream.h"
#include "DisplayFaceStatistics.h"
#include "DisplayForces.h"
#include "Foam.h"
#include "Utils.h"
#include "ViewSettings.h"

// Private Functions
// ======================================================================

void initialize (boost::array<GLfloat, 4>& colors,
		 const boost::array<GLfloat, 4>& values)
{
    copy (values.begin (), values.end (), colors.begin ());
}

boost::shared_ptr<IdBodySelector> idBodySelectorComplement (
    const Foam& foam, const vector<size_t> bodyIds)
{
    Foam::Bodies bodies = foam.GetBodies ();
    vector<size_t> allBodyIds (bodies.size ());
    transform (bodies.begin (), bodies.end (), allBodyIds.begin (),
	       boost::bind (&Body::GetId, _1));
    boost::shared_ptr<IdBodySelector> idBodySelector =
	boost::make_shared<IdBodySelector> (allBodyIds);
    idBodySelector->SetDifference (bodyIds);
    return idBodySelector;
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
    m_displayForces (new DisplayForces (glWidget)),
    m_rotationModel (G3D::Matrix3::identity ()),
    m_scaleRatio (1),
    m_translation (G3D::Vector3::zero ()),
    m_lightingEnabled (false),
    m_selectedLight (LightNumber::LIGHT0),
    m_lightEnabled (0),
    m_lightPositionShown (0),
    m_angleOfView (0),
    m_averageAroundType (AVERAGE_AROUND_NONE),
    m_averageAroundBodyId (INVALID_INDEX),
    m_contextView (false),
    m_forceNetworkShown (true),
    m_forcePressureShown (true),
    m_forceResultShown (true),
    m_contextScaleRatio (1),
    m_contextStationaryType (CONTEXT_AVERAGE_AROUND_NONE),
    m_bodySelector (AllBodySelector::Get ())
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

bool ViewSettings::IsContextDisplayBody (size_t bodyId) const
{
    return m_contextBody.find (bodyId) != m_contextBody.end ();
}

void ViewSettings::CopyColorBar (const ViewSettings& from)
{
    *m_colorBarModel = *from.m_colorBarModel;
}

void ViewSettings::SetColorBarModel (
    const boost::shared_ptr<ColorBarModel>& colorBarModel)
{
    m_colorBarModel = colorBarModel;
    const QImage image = colorBarModel->GetImage ();
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, image.width (),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
}

void ViewSettings::SetBodySelector (
    boost::shared_ptr<AllBodySelector> selector, BodySelectorType::Enum type)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	break;
    case BodySelectorType::ID:
    case BodySelectorType::PROPERTY_VALUE:
	if (type == m_bodySelector->GetType ())
	    m_bodySelector = selector;
    	break;
    case BodySelectorType::COMPOSITE:
	if (type == BodySelectorType::ID)
	    m_bodySelector = boost::static_pointer_cast<CompositeBodySelector> (
		m_bodySelector)->GetPropertyValueSelector ();
	else
	    m_bodySelector = boost::static_pointer_cast<CompositeBodySelector> (
		m_bodySelector)->GetIdSelector ();
	break;
    }
}

void ViewSettings::SetBodySelector (boost::shared_ptr<IdBodySelector> selector)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = selector;
	break;
    case BodySelectorType::ID:
	m_bodySelector = selector;
	break;
    case BodySelectorType::PROPERTY_VALUE:
	m_bodySelector = boost::shared_ptr<BodySelector> (
	    new CompositeBodySelector (
		selector,
		boost::static_pointer_cast<PropertyValueBodySelector> (
		    m_bodySelector)));
	break;
    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->SetSelector (selector);
	break;
    }
}



void ViewSettings::SetBodySelector (
    boost::shared_ptr<PropertyValueBodySelector> selector)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = selector;
	break;
    case BodySelectorType::ID:
	m_bodySelector = boost::shared_ptr<BodySelector> (
	    new CompositeBodySelector (
		boost::static_pointer_cast<IdBodySelector> (m_bodySelector),
		selector));
	break;
    case BodySelectorType::PROPERTY_VALUE:
	m_bodySelector = selector;
	break;
    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->SetSelector (selector);
	break;
    }
}

void ViewSettings::UnionBodySelector (const vector<size_t>& bodyIds)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = boost::make_shared<IdBodySelector> (bodyIds);
	break;

    case BodySelectorType::ID:
    {
	IdBodySelector& selector =
	    *boost::static_pointer_cast<IdBodySelector> (m_bodySelector);
	selector.SetUnion (bodyIds);
	break;
    }

    case BodySelectorType::PROPERTY_VALUE:
    {
	boost::shared_ptr<IdBodySelector> idSelector =
	    boost::make_shared<IdBodySelector> (bodyIds);
	m_bodySelector = boost::make_shared<CompositeBodySelector> (
	    idSelector,
	    boost::static_pointer_cast<PropertyValueBodySelector> (
		m_bodySelector));
	break;
    }

    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->GetIdSelector ()->SetUnion (bodyIds);
	break;
    }
}

void ViewSettings::DifferenceBodySelector (
    const Foam& foam, const vector<size_t>& bodyIds)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
    {
	m_bodySelector = idBodySelectorComplement (foam, bodyIds);
	break;
    }
    case BodySelectorType::ID:
	boost::static_pointer_cast<IdBodySelector> (
	    m_bodySelector)->SetDifference (bodyIds);
	break;

    case BodySelectorType::PROPERTY_VALUE:
    {
	boost::shared_ptr<IdBodySelector> idSelector =
	    idBodySelectorComplement (foam, bodyIds);
	m_bodySelector = boost::make_shared<CompositeBodySelector> (
	    idSelector,
	    boost::static_pointer_cast<PropertyValueBodySelector> (
		m_bodySelector));
	break;
    }

    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->GetIdSelector ()->SetDifference (bodyIds);
	break;
    }
}

void ViewSettings::CopyTransformations (const ViewSettings& from)
{
    m_rotationModel = from.m_rotationModel;
    m_scaleRatio = from.m_scaleRatio;
    m_translation = from.m_translation;
    m_axesOrder = from.m_axesOrder;
}


void ViewSettings::CopySelection (const ViewSettings& other)
{
    m_bodySelector = other.m_bodySelector->Clone ();
}
