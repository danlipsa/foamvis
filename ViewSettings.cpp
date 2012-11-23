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
#include "Foam.h"
#include "OpenGLUtils.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"


// Private Functions
// ======================================================================

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

// Fields
// ======================================================================
const double ViewSettings::STREAMLINE_LENGTH (0.2);
const double ViewSettings::STREAMLINE_STEP_LENGTH (0.005);



// Methods
// ======================================================================
ViewSettings::ViewSettings () :
    m_viewType (ViewType::COUNT),
    m_bodyOrFaceScalar (BodyScalar::PRESSURE),
    m_statisticsType (StatisticsType::AVERAGE),
    m_rotationFocus (G3D::Matrix3::identity ()),
    m_rotationCenterType (ROTATION_CENTER_FOAM),
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
    m_velocityVis (VectorVis::GLYPH),
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
    m_forceTorqueLineWidth (1),
    m_histogramShown (false),
    m_histogramOptions (HistogramType::UNICOLOR_TIME_STEP),
    m_domainClipped (false),
    m_streamlineLength (STREAMLINE_LENGTH),
    m_streamlineStepLength (STREAMLINE_STEP_LENGTH),
    m_timeDisplacement (0.0),
    m_bubblePathsTimeBegin (0),
    m_bubblePathsTimeEnd (0),
    m_seedsShown (false),
    m_kdeSeedsEnabled (false),
    m_kdeValue (0),
    m_kdeMultiplier (3)
{
    setInitialLightParameters ();
    for (size_t i = 0; i < m_averageAroundBodyId.size (); ++i)
	m_averageAroundBodyId[i] = INVALID_INDEX;
}

ViewSettings::~ViewSettings ()
{
}

float ViewSettings::GetVelocityClampingRatio () const
{
    QwtDoubleInterval values = m_velocityOverlayBarModel->GetInterval ();
    double interval = values.maxValue () - values.minValue ();
    QwtDoubleInterval clampValues = 
        m_velocityOverlayBarModel->GetClampInterval ();
    double clampInterval = clampValues.maxValue () - clampValues.minValue ();
    if (clampInterval == 0)
	return numeric_limits<float>::max ();
    else
	return interval / clampInterval;
}


void ViewSettings::setInitialLightParameters ()
{
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
    {
	SetInitialLightParameters (LightNumber::Enum(i));
	m_directionalLightEnabled[i] = true;
    }

    boost::array<boost::array<GLfloat,4>, LightType::COUNT> light = {{
	    // default (0, 0, 0, 1) ambient
	    {{0, 0, 0, 1.0}},
	    // default (1, 1, 1, 1) diffuse
	    {{1.0, 1.0, 1.0, 1.0}},
	    // default (1, 1, 1, 1) specular
	    {{0.0, 0.0, 0.0, 1.0}}
	}};
    for(size_t i = 0; i < LightNumber::COUNT; ++i)
	for (size_t j = 0; j < LightType::COUNT; ++j)
	{
	    boost::array<GLfloat, 4>& src = light[j];
	    copy (src.begin (), src.end (), m_light[i][j].begin ());
	}
}

void ViewSettings::SetLightEnabled (LightNumber::Enum i, bool enabled)
{
    m_lightEnabled[i] = enabled;
    m_lightingEnabled = m_lightEnabled.any ();
}


void ViewSettings::SetInitialLightParameters (LightNumber::Enum i)
{
    m_lightPositionRatio[i] = 1;
    m_rotationLight[i] = G3D::Matrix3::identity ();
}

G3D::Vector3 ViewSettings::GetInitialLightPosition (
    G3D::AABox centeredViewingVolume,
    LightNumber::Enum lightNumber)
{    
    G3D::Vector3 high = centeredViewingVolume.high (), 
	low = centeredViewingVolume.low ();
    G3D::Vector3 nearRectangle[] = {
	G3D::Vector3 (high.x, high.y, high.z),
	G3D::Vector3 (low.x, high.y, high.z),
	G3D::Vector3 (low.x, low.y, high.z),
	G3D::Vector3 (high.x, low.y, high.z),
    };
    return nearRectangle[lightNumber];
}


void ViewSettings::SetGlLightParameters (
    G3D::AABox centeredViewingVolume) const
{
    if (m_lightingEnabled)
	glEnable (GL_LIGHTING);
    else
	glDisable (GL_LIGHTING);
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
	SetGlLightParameters (LightNumber::Enum (i), centeredViewingVolume);
}

void ViewSettings::SetGlLightParameters (
    LightNumber::Enum lightNumber, 
    G3D::AABox centeredViewingVolume) const
{
    if (IsLightEnabled (lightNumber))
    {
	glEnable(GL_LIGHT0 + lightNumber);
	G3D::Vector3 initialLightPosition = GetInitialLightPosition (
	    centeredViewingVolume, lightNumber);
	G3D::Vector3 lp = initialLightPosition * 
	    GetLightPositionRatio (lightNumber);
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
    else
	glDisable (GL_LIGHT0 + lightNumber);
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
}

void ViewSettings::SetOverlayBarModel (
    const boost::shared_ptr<ColorBarModel>& colorBarModel)
{
    m_velocityOverlayBarModel = colorBarModel;
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

void ViewSettings::UnionBodySelector (size_t bodyId)
{
    UnionBodySelector (vector<size_t> (1, bodyId));
}

void ViewSettings::UnionBodySelector (const vector<size_t>& bodyIds)
{
    if (bodyIds.empty ())
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
    if (bodyIds.empty ())
	return;
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
    {
	if (! bodyIds.empty ())
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
    Q_EMIT SelectionChanged ();
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


int ViewSettings::SetCurrentTime (size_t time)
{
    int direction = time - GetCurrentTime ();
    m_currentTime = time;
    return direction;
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
	    (*foam.FindBody (bodyId))->GetCenter ();
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

void ViewSettings::SetSimulation (int i, const Simulation& simulation, 
				  G3D::Vector3 viewingVolumeCenter, 
				  bool t1sShiftLower)
{
    int rotation2D = simulation.GetRotation2D ();
    size_t reflexionAxis = simulation.GetReflectionAxis ();
    setSimulationIndex (i);
    SetAxesOrder (simulation.Is2D () ? 
		  (rotation2D == 0 ? AxesOrder::TWO_D :
		   (rotation2D == 90 ? AxesOrder::TWO_D_ROTATE_LEFT90 : 
		    ((reflexionAxis == 1) ? 
		     AxesOrder::TWO_D_ROTATE_RIGHT90_REFLECTION :
		     AxesOrder::TWO_D_ROTATE_RIGHT90))): AxesOrder::THREE_D);
    SetT1sShiftLower (t1sShiftLower);
    SetScaleCenter (viewingVolumeCenter.xy ());
    SetRotationCenter (viewingVolumeCenter);
    setTimeSteps (simulation.GetTimeSteps ());
    SetBubblePathsTimeEnd (simulation.GetTimeSteps ());
}

void ViewSettings::RotateAndTranslateAverageAround (
    size_t timeStep, int direction, RotateAndTranslateOperation op) const
{
    const ObjectPosition posBegin = GetAverageAroundPosition (0);
    const ObjectPosition posCurrent = GetAverageAroundPosition (timeStep);
    float angleRadians = posCurrent.m_angleRadians - posBegin.m_angleRadians;
    G3D::Vector3 translation;
    if (op == TRANSLATE)
    {
	translation = posBegin.m_rotationCenter - posCurrent.m_rotationCenter;
        if (direction > 0)
            glTranslate (translation);
    }
    if (angleRadians != 0)
    {
	G3D::Vector3 rotationCenter = posCurrent.m_rotationCenter;
	glTranslate (rotationCenter);
	float angleDegrees =  G3D::toDegrees (angleRadians);
	//cdbg << "angle degrees = " << angleDegrees << endl;
	angleDegrees = direction > 0 ? angleDegrees : - angleDegrees;
	glRotatef (angleDegrees, 0, 0, 1);
	glTranslate (-rotationCenter);
    }
    if (op == TRANSLATE && direction < 0)
        glTranslate (-translation);
}

bool ViewSettings::HasHistogramOption (HistogramType::Option option) const
{
    return m_histogramOptions.testFlag (option);
}

void ViewSettings::SetHistogramOption (HistogramType::Option option, bool on)
{
    if (on)
	m_histogramOptions |= option;
    else
	ResetHistogramOption (option);
}

void ViewSettings::ResetHistogramOption (HistogramType::Option option)
{
    if (HasHistogramOption (option))
	m_histogramOptions ^= option;
}

string ViewSettings::GetTitle (ViewNumber::Enum viewNumber) const
{
    ostringstream ostr;
    ostr << "View " << viewNumber << " - "
	 << ViewType::ToString (GetViewType ()) << " - "
	 << FaceScalar::ToString (GetBodyOrFaceScalar ()) << " - "
	 << "Time " << GetCurrentTime ();
    return ostr.str ();
}




ColorBarType::Enum ViewSettings::GetColorBarType () const
{
    ViewType::Enum viewType = GetViewType ();
    size_t property = GetBodyOrFaceScalar ();
    StatisticsType::Enum statisticsType = GetStatisticsType ();
    return GetColorBarType (viewType, property, statisticsType);
}

ColorBarType::Enum ViewSettings::GetColorBarType (
    ViewType::Enum viewType, size_t property,
    StatisticsType::Enum statisticsType)
{
    switch (viewType)
    {
    case ViewType::T1S_KDE:
	return ColorBarType::T1S_KDE;
    case ViewType::AVERAGE:
	if (statisticsType == StatisticsType::COUNT)
	    return ColorBarType::STATISTICS_COUNT;
    case ViewType::FACES:
	if (property == FaceScalar::DMP_COLOR)
	    return ColorBarType::NONE;
    case ViewType::CENTER_PATHS:
	return ColorBarType::PROPERTY;
    default:
	return ColorBarType::NONE;
    }
}
