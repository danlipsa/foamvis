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
#include "Debug.h"
#include "Foam.h"
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
const pair<float,float> ViewSettings::ALPHA_RANGE (0, 1);
const pair<float,float> ViewSettings::TENSOR_LINE_WIDTH_EXP2 (0, 3);
const pair<float,float> ViewSettings::T1_SIZE (0.05, 1);
const pair<float,float> ViewSettings::EDGE_RADIUS_RATIO (0.0, 1.0);
const size_t ViewSettings::MAX_RADIUS_MULTIPLIER = 5;


// Methods
// ======================================================================
ViewSettings::ViewSettings () :
    m_viewType (ViewType::COUNT),
    m_bodyOrFaceScalar (BodyScalar::PRESSURE),
    m_statisticsType (StatisticsType::AVERAGE),
    m_rotationFocus (G3D::Matrix3::identity ()),
    m_rotationCenterType (ROTATION_CENTER_FOAM),
    m_scaleRatio (1),
    m_seedShown (false),
    m_glyphSeedsCount (50),
    m_kdeSeedEnabled (false),
    m_seedScaleRatio (1),
    m_contextScaleRatio (1),
    m_lightingEnabled (false),
    m_selectedLight (LightNumber::LIGHT0),
    m_lightEnabled (0),
    m_directionalLightEnabled (0),    
    m_lightPositionShown (0),
    m_angleOfView (0),
    m_axesOrder (AxisOrderName::COUNT),
    m_cameraDistance (0),
    m_scalarShown (true),
    m_scalarContext (false),
    m_averageAround (false),
    m_averageAroundRotationShown (false),
    m_differenceBodyId (INVALID_INDEX),
    m_contextView (false),
    m_deformationShown (false),
    m_velocityShown (false),
    m_velocityGlyphSameSize (false),
    m_velocityVis (VectorVis::GLYPH),
    m_bodySelector (AllBodySelector::Get ()),
    m_selectionContextShown (true),
    m_contextAlpha (0.01),
    m_objectAlpha (0.05),
    m_t1KDEIsosurfaceAlpha (1),
    m_centerPathHidden (false),
    m_simulationIndex (0),
    m_time (0),
    m_timeSteps (0),
    m_timeWindow (0),
    m_t1ShiftLower (false),
    m_deformationSize (1),
    m_deformationLineWidth (1),
    m_velocityLineWidth (1),
    m_velocityColorMapped (true),
    m_forceRatio (1),
    m_torqueDistance (1),
    m_histogramShown (false),
    m_histogramOptions (HistogramType::UNICOLOR_TIME_STEP),
    m_domainClipped (false),
    m_streamlineLength (STREAMLINE_LENGTH),
    m_streamlineStepLength (STREAMLINE_STEP_LENGTH),
    m_timeDisplacement (0.0),
    m_bubblePathsTimeBegin (0),
    m_bubblePathsTimeEnd (0),
    m_kdeSeedingValue (0),
    m_kdeMultiplier (3),
    m_T1KDEKernelBoxShown (false),
    m_T1KDESigmaInBubbleDiameter (1),
    m_t1KDEIsosurfaceValue (0.1),
    m_t1Size ((T1_SIZE.first + T1_SIZE.second) / 4),
    m_t1Shown (false),
    m_t1AllTimeSteps (false),
    m_onePixelInObjectSpace (0),
    m_axesShown (false),
    m_boundingBoxSimulationShown (false),
    m_torusDomainShown (false),
    m_scalarContourShown (false),
    m_dmpTransformShown (false),
    m_edgeWidth (0),
    m_edgeRadius (0),
    m_edgeRadiusRatio (0),
    m_arrowHeadRadius (0),
    m_arrowHeadHeight (0),
    m_centerPathTubeUsed (true),
    m_centerPathLineUsed (false),
    m_edgeVis (EdgeVis::EDGE_NORMAL),
    m_clipPlaneShown (false),
    m_clipPlaneNormal (0, 0, -1)
{
    setInitialLightParameters ();
    for (size_t i = 0; i < m_averageAroundBodyId.size (); ++i)
	m_averageAroundBodyId[i] = INVALID_INDEX;
    fill (m_forceShown.begin (), m_forceShown.end (), false);
    fill (m_torqueShown.begin (), m_torqueShown.end (), false);
}

ViewSettings::~ViewSettings ()
{
}

QColor ViewSettings::GetBubblePathsContextColor () const
{
    QColor returnColor (Qt::black);
    returnColor.setAlphaF (GetContextAlpha ());
    return returnColor;
}


float ViewSettings::GetVelocityInverseClampMaxRatio () const
{
    float clampMaxRatio = m_colorMapVelocity->GetClampMaxRatio ();
    if (clampMaxRatio == 0)
        return numeric_limits<float>::max ();
    else
        return 1 / clampMaxRatio;
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
	    {{0, 0, 0, 1.0}}
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

void ViewSettings::SetLight (
    LightNumber::Enum lightNumber, LightType::Enum lightType, 
    size_t colorIndex, GLfloat color)
{
    m_light[lightNumber][lightType][colorIndex] = color;
}

void ViewSettings::SetLight (
    LightNumber::Enum lightNumber, LightType::Enum lightType, 
    const boost::array<GLfloat,4>& color)
{
    m_light[lightNumber][lightType] = color;
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


bool ViewSettings::IsContextDisplayBody (size_t bodyId) const
{
    return m_contextBody.find (bodyId) != m_contextBody.end ();
}

void ViewSettings::CopyColorMapScalar (const ViewSettings& from)
{
    m_colorMapScalar->ColorMapCopy (*from.m_colorMapScalar);
}

void ViewSettings::CopyColorMapVelocity (const ViewSettings& from)
{
    m_colorMapVelocity->ColorMapCopy (*from.m_colorMapVelocity);
}

void ViewSettings::CopyColorMapVelocityFromScalar ()
{
    m_colorMapVelocity->ColorMapCopy (*m_colorMapScalar);
}

void ViewSettings::SetColorMapScalar (
    const boost::shared_ptr<ColorBarModel>& colorBarModel)
{
    m_colorMapScalar = colorBarModel;
}

void ViewSettings::SetColorMapVelocity (
    const boost::shared_ptr<ColorBarModel>& colorBarModel)
{
    m_colorMapVelocity = colorBarModel;
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
		m_bodySelector)->GetValueSelector ();
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
		boost::static_pointer_cast<ValueBodySelector> (
		    m_bodySelector)));
	break;
    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->SetSelector (selector);
	break;
    }
}



void ViewSettings::SetBodySelector (
    boost::shared_ptr<ValueBodySelector> selector)
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
	    boost::static_pointer_cast<ValueBodySelector> (
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
	    boost::static_pointer_cast<ValueBodySelector> (
		m_bodySelector));
	break;
    }

    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->GetIdSelector ()->SetDifference (bodyIds);
	break;
    }
}

void ViewSettings::CopySelection (const ViewSettings& other)
{
    m_bodySelector = other.m_bodySelector->Clone ();
    Q_EMIT SelectionChanged ();
}

size_t ViewSettings::GetBodyOrOtherScalar () const
{
    return GetViewType () == ViewType::T1_KDE ? 
        static_cast<size_t> (OtherScalar::T1_KDE) : m_bodyOrFaceScalar;
}

G3D::Matrix3 ViewSettings::GetRotationForAxisOrder (const Foam& foam) const
{
    switch (m_axesOrder)
    {
    case AxisOrderName::TWO_D:
	return G3D::Matrix3::identity ();
    case AxisOrderName::TWO_D_TIME_DISPLACEMENT:
	return getRotation2DTimeDisplacement ();
    case AxisOrderName::TWO_D_ROTATE_RIGHT90:
	return getRotation2DRight90 ();
    case AxisOrderName::TWO_D_ROTATE_RIGHT90_REFLECTION:
	return getRotation2DRight90Reflection ();
    case AxisOrderName::TWO_D_ROTATE_LEFT90:
	return getRotation2DLeft90 ();
    case AxisOrderName::THREE_D:
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
    case AxisOrderName::TWO_D_ROTATE_RIGHT90:
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
    if (IsDmpTransformShown ())
        rotation = rotation * 
            foam.GetViewMatrix ().approxCoordinateFrame ().rotation;
    return rotation;
}


int ViewSettings::SetTime (size_t time)
{
    int direction = time - GetTime ();
    m_time = time;
    return direction;
}

void ViewSettings::SetSimulation (int i, const Simulation& simulation, 
				  G3D::Vector3 viewingVolumeCenter)
{
    int rotation2D = simulation.GetRotation2D ();
    size_t reflexionAxis = simulation.GetReflectionAxis ();
    setSimulationIndex (i);
    SetAxisOrder (simulation.Is2D () ? 
		  (rotation2D == 0 ? AxisOrderName::TWO_D :
		   (rotation2D == 90 ? AxisOrderName::TWO_D_ROTATE_LEFT90 : 
		    ((reflexionAxis == 1) ? 
		     AxisOrderName::TWO_D_ROTATE_RIGHT90_REFLECTION :
		     AxisOrderName::TWO_D_ROTATE_RIGHT90))): AxisOrderName::THREE_D);
    SetT1sShiftLower (simulation.GetT1ShiftLower ());
    SetScaleCenter (viewingVolumeCenter.xy ());
    SetRotationCenter (viewingVolumeCenter);
    setTimeSteps (simulation.GetTimeSteps ());
    SetBubblePathsTimeEnd (simulation.GetTimeSteps ());
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
    ostr << "View " << viewNumber << " - ";
    ostr << "Time " << GetTime ();
    return ostr.str ();
}

ColorMapScalarType::Enum ViewSettings::GetColorMapScalarType () const
{
    ViewType::Enum viewType = GetViewType ();
    size_t property = GetBodyOrOtherScalar ();
    StatisticsType::Enum statisticsType = GetStatisticsType ();
    return GetColorMapScalarType (viewType, property, statisticsType);
}

ColorMapScalarType::Enum ViewSettings::GetColorMapScalarType (
    ViewType::Enum viewType, size_t property,
    StatisticsType::Enum statisticsType)
{
    switch (viewType)
    {
    case ViewType::T1_KDE:
	return ColorMapScalarType::T1_KDE;

    case ViewType::AVERAGE:
	if (statisticsType == StatisticsType::COUNT)
	    return ColorMapScalarType::STATISTICS_COUNT;
        return ColorMapScalarType::PROPERTY;

    case ViewType::FACES:
	if (property == OtherScalar::DMP_COLOR)
	    return ColorMapScalarType::NONE;
        return ColorMapScalarType::PROPERTY;

    case ViewType::BUBBLE_PATHS:
	return ColorMapScalarType::PROPERTY;
    default:
	return ColorMapScalarType::NONE;
    }
}

size_t ViewSettings::GetLinkedTimeInterval (size_t eventIndex) const
{
    RuntimeAssert (eventIndex <= m_linkedTimeEvent.size (),
                   "Event index out of range: ", eventIndex);
    size_t timeInterval;    
    if (eventIndex == m_linkedTimeEvent.size ())
        timeInterval = GetTimeSteps ();
    else
        timeInterval = m_linkedTimeEvent[eventIndex] + 1;
    if (eventIndex != 0)
        timeInterval -= (m_linkedTimeEvent[eventIndex - 1] + 1);
    return timeInterval;
}

void ViewSettings::AddLinkedTimeEvent (size_t timeEvent)
{
    // search for the insertion location
    size_t i;
    for (i = 0; i < m_linkedTimeEvent.size (); ++i)
    {
        if (m_linkedTimeEvent[i] == timeEvent)
            ThrowException ("Event already in the list");
        if (m_linkedTimeEvent[i] > timeEvent)
            break;
    }

    // move elements to the right
    m_linkedTimeEvent.resize (m_linkedTimeEvent.size () + 1);
    size_t toStore = timeEvent;
    for (size_t j = i; j < m_linkedTimeEvent.size (); ++j)
    {
        size_t temp = m_linkedTimeEvent[j];
        m_linkedTimeEvent[j] = toStore;
        toStore = temp;
    }
}

void ViewSettings::ResetLinkedTimeEvents ()
{
    m_linkedTimeEvent.resize (0);
}

void ViewSettings::SetArrowParameters (float onePixelInObjectSpace)
{
    SetArrowParameters (
        onePixelInObjectSpace,
        &m_edgeRadius, &m_arrowHeadRadius, &m_arrowHeadHeight, 
        GetEdgeRadiusRatio (),
        &m_edgeWidth);
}

void ViewSettings::SetArrowParameters (
    float onePixelInObjectSpace,
    float* edgeRadius, float* arrowHeadRadius, float* arrowHeadHeight, 
    float edgeRadiusRatio,float* ew)
{
    float edgeWidth = (MAX_RADIUS_MULTIPLIER - 1) * edgeRadiusRatio + 1;
    if (ew != 0)
        *ew = edgeWidth;
    *edgeRadius = onePixelInObjectSpace * edgeWidth;
    *arrowHeadRadius = 4 * (*edgeRadius);
    *arrowHeadHeight = 11 * (*edgeRadius);
}

void ViewSettings::SetDimension (Dimension::Enum dimension)
{
    SetLightEnabled (LightNumber::LIGHT0, dimension == Dimension::D3D);
    SetEdgeRadiusRatio (dimension == Dimension::D3D? 0.5 : 0);
    Q_EMIT ViewChanged ();
}
