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
#include "Debug.h"
#include "ScalarAverage.h"
#include "ForceAverage.h"
#include "Foam.h"
#include "OpenGLUtils.h"
#include "Simulation.h"
#include "T1sPDE.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"
#include "VectorAverage.h"

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
    m_bodyOrFaceProperty (BodyProperty::PRESSURE),
    m_statisticsType (StatisticsType::AVERAGE),
    m_listCenterPaths (0),
    m_scalarAverage (new ScalarAverage (glWidget)),
    m_t1sPDE (new T1sPDE (glWidget)),
    m_deformationAverage (
	new TensorAverage (glWidget, m_scalarAverage->GetFbos ())),
    m_velocityAverage (
	new VectorAverage (glWidget, m_scalarAverage->GetFbos ())),
    m_forceAverage (new ForceAverage (glWidget)),
    m_rotationFocus (G3D::Matrix3::identity ()),
    m_scaleRatio (1),
    m_gridScaleRatio (1),
    m_contextScaleRatio (1),
    m_lightingEnabled (false),
    m_selectedLight (LightNumber::LIGHT0),
    m_lightEnabled (0),
    m_directionalLightEnabled (0),    
    m_lightPositionShown (0),
    m_angleOfView (0),
    m_axesOrder (AxesOrder::COUNT),
    m_cameraDistance (0),
    m_averageAround (false),
    m_differenceBodyId (INVALID_INDEX),
    m_contextView (false),
    m_forceNetworkShown (false),
    m_forcePressureShown (false),
    m_forceResultShown (false),
    m_torqueNetworkShown (false),
    m_torquePressureShown (false),
    m_torqueResultShown (false),
    m_forceDifferenceShown (false),
    m_deformationShown (false),
    m_velocityShown (false),
    m_averageAroundRotationShown (false),
    m_bodySelector (AllBodySelector::Get ()),
    m_selectionContextShown (true),
    m_centerPathHidden (false),
    m_simulationIndex (0),
    m_currentTime (0),
    m_t1sShiftLower (false),
    m_syncViewTimeBegin (0),
    m_syncViewTimeEnd (0),
    m_deformationSize (1),
    m_deformationLineWidth (1),
    m_velocityLineWidth (1),
    m_forceTorqueSize (1),
    m_torqueDistance (1),
    m_forceTorqueLineWidth (1)
{    
    initTexture ();
    initList ();
    setInitialLightParameters ();
    for (size_t i = 0; i < m_averageAroundBodyId.size (); ++i)
	m_averageAroundBodyId[i] = INVALID_INDEX;
}

ViewSettings::~ViewSettings ()
{
    glDeleteTextures (2, m_colorBarTexture);
    glDeleteLists (m_listCenterPaths, 1);
}

float ViewSettings::GetVelocityClampingRatio () const
{
    QwtDoubleInterval values = m_velocityOverlayBarModel->GetInterval ();
    double interval = values.maxValue () - values.minValue ();
    QwtDoubleInterval clampValues = m_velocityOverlayBarModel->GetClampValues ();
    double clampInterval = clampValues.maxValue () - clampValues.minValue ();
    if (clampInterval == 0)
	return numeric_limits<float>::max ();
    else
	return interval / clampInterval;
}


void ViewSettings::initTexture ()
{
    glGenTextures (2, m_colorBarTexture);
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture (GL_TEXTURE_1D, GetOverlayBarTexture ());
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
	SetInitialLightParameters (LightNumber::Enum(i));
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


void ViewSettings::SetInitialLightParameters (LightNumber::Enum i)
{
    m_lightPositionRatio[i] = 1;
    m_rotationLight[i] = G3D::Matrix3::identity ();
}

void ViewSettings::SetLightParameters (
    LightNumber::Enum lightNumber, const G3D::Vector3& initialLightPosition)
{
    if (IsLightEnabled (lightNumber))
    {
	G3D::Vector3 lp = initialLightPosition * 
	    GetLightNumberRatio (lightNumber);
	if (IsDirectionalLightEnabled (lightNumber))
	{
	    glPushMatrix ();
	    glLoadIdentity ();
	    glMultMatrix (GetRotationLight (lightNumber));
	    glLightf(GL_LIGHT0 + lightNumber, GL_SPOT_CUTOFF, 180);
	    boost::array<GLfloat, 4> lightPosition = {{lp.x, lp.y, lp.z, 0}};
	    glLightfv(GL_LIGHT0 + lightNumber, GL_POSITION, &lightPosition[0]);
	    glPopMatrix ();
	}
	else
	{
	    glLightf(GL_LIGHT0 + lightNumber, GL_SPOT_CUTOFF, 15);
	    glPushMatrix ();
	    glLoadIdentity ();	    
	    glTranslated (0, 0, - m_cameraDistance);
	    glMultMatrix (GetRotationLight (lightNumber));
	    boost::array<GLfloat, 3> lightDirection = {{-lp.x, -lp.y, -lp.z}};
	    glLightfv(GL_LIGHT0 + lightNumber, 
		      GL_SPOT_DIRECTION, &lightDirection[0]);
	    GLfloat lightPosition[] = {lp.x, lp.y, lp.z, 1};
	    glLightfv(GL_LIGHT0 + lightNumber, GL_POSITION, lightPosition);
	    glPopMatrix ();
	}
	
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

void ViewSettings::SetLight (
    LightNumber::Enum lightNumber, LightType::Enum lightType, 
    size_t colorIndex, GLfloat color)
{
    m_light[lightNumber][lightType][colorIndex] = color;
    glLightfv (GL_LIGHT0 + lightNumber, LightType::ToOpenGL (lightType), 
	       &m_light[lightNumber][lightType][0]);
}

void ViewSettings::SetLight (
    LightNumber::Enum lightNumber, LightType::Enum lightType, 
    const boost::array<GLfloat,4>& color)
{
    m_light[lightNumber][lightType] = color;
    glLightfv (GL_LIGHT0 + lightNumber, LightType::ToOpenGL (lightType), 
	       &m_light[lightNumber][lightType][0]);
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
    if (colorBarModel)
    {
	const QImage image = colorBarModel->GetImage ();
	glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
	glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, image.width (),
		      0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
    }
}

void ViewSettings::SetOverlayBarModel (
    const boost::shared_ptr<ColorBarModel>& colorBarModel)
{
    m_velocityOverlayBarModel = colorBarModel;
    if (colorBarModel)
    {
	const QImage image = colorBarModel->GetImage ();
	glBindTexture (GL_TEXTURE_1D, GetOverlayBarTexture ());
	glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, image.width (),
		      0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
    }
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
    if (bodyIds.size () == 0)
	return;
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
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
    if (bodyIds.size () == 0)
	return;
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
    {
	if (bodyIds.size () != 0)
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

void ViewSettings::CopyTransformation (const ViewSettings& from)
{
    m_rotationFocus = from.m_rotationFocus;
    m_scaleRatio = from.m_scaleRatio;
    m_translation = from.m_translation;
    m_axesOrder = from.m_axesOrder;
}


void ViewSettings::CopySelection (const ViewSettings& other)
{
    m_bodySelector = other.m_bodySelector->Clone ();
}

void ViewSettings::AverageInit (ViewNumber::Enum viewNumber)
{
    GetScalarAverage ().AverageInit (viewNumber);
    GetDeformationAverage ().AverageInit (viewNumber);
    GetVelocityAverage ().AverageInit (viewNumber);
    GetForceAverage ().AverageInit (viewNumber);
    GetT1sPDE ().AverageInit (viewNumber);
}

void ViewSettings::AverageSetTimeWindow (size_t timeSteps)
{
    GetScalarAverage ().AverageSetTimeWindow (timeSteps);
    GetDeformationAverage ().AverageSetTimeWindow (timeSteps);
    GetVelocityAverage ().AverageSetTimeWindow (timeSteps);
    GetForceAverage ().AverageSetTimeWindow (timeSteps);
}

void ViewSettings::AverageStep (ViewNumber::Enum viewNumber, int direction)
{
    switch (GetViewType ())
    {
    case ViewType::FACES_STATISTICS:
	GetScalarAverage ().AverageStep (viewNumber, direction);
	GetDeformationAverage ().AverageStep (viewNumber, direction);
	GetVelocityAverage ().AverageStep (viewNumber, direction);
	GetForceAverage ().AverageStep (viewNumber, direction);
	break;
	
    case ViewType::T1S_PDE:
	GetT1sPDE ().AverageStep (viewNumber, direction);
	break;
    default:
	break;
    }
}

void ViewSettings::AverageRotateAndDisplay (
    ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    switch (GetViewType ())
    {
    case ViewType::FACES_STATISTICS:
	GetScalarAverage ().AverageRotateAndDisplay (
	    viewNumber, displayType, rotationCenter, angleDegrees);
	if (IsDeformationShown ())
	    GetDeformationAverage ().AverageRotateAndDisplay (
		viewNumber, displayType, rotationCenter, angleDegrees);
	if (IsVelocityShown ())
	    GetVelocityAverage ().AverageRotateAndDisplay (
		viewNumber, displayType, rotationCenter, angleDegrees);	    
	break;
	
    case ViewType::T1S_PDE:
	GetT1sPDE ().AverageRotateAndDisplay (
	    viewNumber, displayType, rotationCenter, angleDegrees);
	break;
    default:
	break;
    }
}

void ViewSettings::AverageRelease ()
{
    GetScalarAverage ().AverageRelease ();
    GetDeformationAverage ().AverageRelease ();
    GetVelocityAverage ().AverageRelease ();
    GetT1sPDE ().AverageRelease ();
}

G3D::Matrix3 ViewSettings::GetRotationForAxesOrder (const Foam& foam) const
{
    switch (m_axesOrder)
    {
    case AxesOrder::TWO_D:
	return G3D::Matrix3::identity ();
    case AxesOrder::TWO_D_TIME_DISPLACEMENT:
	return getRotation2DTimeDisplacement ();
    case AxesOrder::TWO_D_ROTATE_RIGHT90:
	return getRotation2DRight90 ();
    case AxesOrder::TWO_D_ROTATE_RIGHT90_REFLECTION:
	return getRotation2DRight90Reflection ();
    case AxesOrder::TWO_D_ROTATE_LEFT90:
	return getRotation2DLeft90 ();
    case AxesOrder::THREE_D:
	return getRotation3D (foam);
    default:
	ThrowException ("Invalid axes order: ", m_axesOrder);
	return G3D::Matrix3::identity ();
    }
}

float ViewSettings::AngleDisplay (float angle) const
{
    switch (m_axesOrder)
    {
    case AxesOrder::TWO_D_ROTATE_RIGHT90:
	if (angle != 0)
	    return - angle;
    default:
	return angle;
    }    
}


G3D::Matrix3 ViewSettings::getRotation2DTimeDisplacement ()
{
    /**
     *  y        z
     *    x ->     x
     * z        -y
     */
    return G3D::Matrix3 (1, 0, 0,  0, 0, 1,  0, -1, 0);
}

G3D::Matrix3 ViewSettings::getRotation2DRight90 ()
{
    /**
     *  y       -x
     *    x ->     y
     * z        z
     */
    return G3D::Matrix3 (0, 1, 0,  -1, 0, 0,  0, 0, 1);
}

G3D::Matrix3 ViewSettings::getRotation2DRight90Reflection ()
{
    /**
     *  y       -x
     *    x ->     y
     * z        z
     */
    return G3D::Matrix3 (0, -1, 0,  -1, 0, 0,  0, 0, 1);
}


G3D::Matrix3 ViewSettings::getRotation2DLeft90 ()
{
    /**
     *  y       x
     *    x ->     -y
     * z        z
     */
    return G3D::Matrix3 (0, -1, 0,  1, 0, 0,  0, 0, 1);
}



G3D::Matrix3 ViewSettings::getRotation3D (const Foam& foam) const
{
    /**
     *  y        z
     *    x ->     y
     * z        x
     */
    const static G3D::Matrix3 evolverAxes (0, 1, 0,  0, 0, 1,  1, 0, 0);
    G3D::Matrix3 rotation (evolverAxes);
    rotation = rotation * 
	foam.GetViewMatrix ().approxCoordinateFrame ().rotation;
    return rotation;
}

size_t ViewSettings::GetCurrentTime () const
{
    return m_currentTime;
}


void ViewSettings::SetCurrentTime (size_t time, ViewNumber::Enum viewNumber)
{
    int direction = time - GetCurrentTime ();
    if (direction == 0)
	return;
    m_currentTime = time;
    AverageStep (viewNumber, direction);
}

void ViewSettings::SetAverageAroundPositions (const Simulation& simulation)
{
    m_averageAroundPositions.resize (simulation.GetTimeSteps ());
    for (size_t i = 0; i < m_averageAroundPositions.size (); ++i)
	m_averageAroundPositions[i] = 
	    simulation.GetFoam (i).GetDmpObjectPosition ();
}

void ViewSettings::SetAverageAroundPositions (
    const Simulation& simulation, size_t bodyId)
{
    m_averageAroundPositions.resize (simulation.GetTimeSteps ());
    for (size_t i = 0; i < m_averageAroundPositions.size (); ++i)
    {
	ObjectPosition& objectPosition = m_averageAroundPositions[i];
	const Foam& foam = simulation.GetFoam (i);
	objectPosition.m_angleRadians = 0;
	objectPosition.m_rotationCenter = 
	    (*foam.FindBody (bodyId))->GetCenter ().xy ();
    }
}

void ViewSettings::SetAverageAroundPositions (
    const Simulation& simulation,
    size_t bodyId, size_t secondBodyId)
{
    G3D::Vector2 beginAxis = 
	simulation.GetFoam (0).GetAverageAroundAxis (bodyId, secondBodyId);
    // the angle for i = 0 is already set to 0. Trying to calculate will results 
    // in acosValue slightly greater than 1, a Nan and then an error 
    // in gluUnProject.
    for (size_t i = 1; i < m_averageAroundPositions.size (); ++i)
    {
	ObjectPosition& objectPosition = m_averageAroundPositions[i];
	const Foam& foam = simulation.GetFoam (i);

	G3D::Vector2 currentAxis = 
	    foam.GetAverageAroundAxis (bodyId, secondBodyId);
	float acosValue = currentAxis.direction ().dot (beginAxis.direction ());
	float angleRadians = acos (acosValue);
	objectPosition.m_angleRadians = - angleRadians;
    }
}

