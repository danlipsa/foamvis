/**
 * @file   Settings.cpp
 * @author Dan R. Lipsa
 * @date 17 July 2012
 *
 * Definitions for the view settings
 */

#include "ColorBarModel.h"
#include "Debug.h"
#include "Edge.h"
#include "Settings.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"
#include "T1sPDE.h"

// Private functions and classes
// ======================================================================

G3D::AABox AdjustXOverYRatio (const G3D::AABox& box, double xOverY)
{
    G3D::Vector3 center = box.center ();
    G3D::Vector3 low, high;
    if (xOverY >= 1)
    {
	double extentX = xOverY * box.extent ().y;
	low = box.low ();
	low.x = center.x - extentX / 2;
	high = box.high ();
	high.x = center.x + extentX / 2;
    }
    else
    {
	double extentY = box.extent ().x / xOverY;
	low = box.low ();
	low.y = center.y - extentY / 2;
	high = box.high ();
	high.y = center.y + extentY / 2;
    }
    G3D::AABox result (low, high);
    return result;
}

G3D::AABox ExtendAlongZFor3D (
    const G3D::AABox& b, double scaleRatio)
{
    if (scaleRatio > 1)
    {
	double zExtent2 = b.extent ().z * scaleRatio / 2;
	double zCenter = b.center ().z;
	G3D::Vector3 low = b.low (), high = b.high ();
	return G3D::AABox (
	    G3D::Vector3 (low.x, low.y, zCenter - zExtent2),
	    G3D::Vector3 (high.x, high.y, zCenter + zExtent2));
    }
    else
	return b;
}


// Methods
// ======================================================================
const pair<float,float> Settings::CONTEXT_ALPHA (0.05, 0.5);
const size_t Settings::QUADRIC_SLICES = 8;
const size_t Settings::QUADRIC_STACKS = 1;


Settings::Settings (const Simulation& simulation, float xOverY) :
    m_contextAlpha (CONTEXT_ALPHA.first),
    m_edgeRadius (0),
    m_edgeWidth (0),
    m_edgeRadiusRatio (0),
    m_arrowBaseRadius (0),
    m_arrowHeight (0),
    m_edgesTessellationShown (false),
    m_constraintsShown (true),
    m_constraintPointsShown (false),
    m_viewNumber (ViewNumber::VIEW0),
    m_timeLinkage (TimeLinkage::LINKED),
    m_linkedTime (0),
    m_viewCount (ViewCount::ONE),
    m_viewLayout (ViewLayout::HORIZONTAL),
    m_missingPressureShown (true),
    m_missingVolumeShown (true),
    m_objectVelocityShown (false),
    m_centerPathTubeUsed (true),
    m_centerPathLineUsed (false)
{
    initViewSettings (simulation, xOverY);
    initEndTranslationColor ();
}

void Settings::SetEdgeArrow (float onePixelInObjectSpace)
{
    const int maxRadiusMultiplier = 5;

    float r = onePixelInObjectSpace;
    float R = maxRadiusMultiplier * r;

    SetEdgeRadius ((R - r) * GetEdgeRadiusRatio () + r);
    SetEdgeWidth ((maxRadiusMultiplier - 1) * GetEdgeRadiusRatio () + 1);
    SetArrowBaseRadius (4 * GetEdgeRadius ());
    SetArrowHeight (11 * GetEdgeRadius ());
}

const QColor& Settings::GetEndTranslationColor (
    const G3D::Vector3int16& di) const
{
    EndLocationColor::const_iterator it = m_endTranslationColor.find (di);
    RuntimeAssert (it != m_endTranslationColor.end (),
		   "Invalid domain increment ", di);
    return (*it).second;
}

void Settings::initEndTranslationColor ()
{
    const int DOMAIN_INCREMENT_COLOR[] = {255, 0, 255};
    for (size_t i = 0;
	 i < Edge::DOMAIN_INCREMENT_POSSIBILITIES *
	     Edge::DOMAIN_INCREMENT_POSSIBILITIES *
	     Edge::DOMAIN_INCREMENT_POSSIBILITIES; i++)
    {
	G3D::Vector3int16 di = Edge::IntToLocation (i);
	QColor color (
	    DOMAIN_INCREMENT_COLOR[di.x + 1],
	    DOMAIN_INCREMENT_COLOR[di.y + 1],
	    DOMAIN_INCREMENT_COLOR[di.z + 1]);
	m_endTranslationColor[di] = color;
    }
    m_endTranslationColor[G3D::Vector3int16(0,0,0)] = QColor(0,0,0);
}

void Settings::initViewSettings (const Simulation& simulation, float xOverY)
{
    ViewNumber::Enum viewNumber (ViewNumber::VIEW0);
    BOOST_FOREACH (boost::shared_ptr<ViewSettings>& vs, m_viewSettings)
    {
	vs = boost::make_shared <ViewSettings> (*this);
	vs->SetViewType (ViewType::FACES);
	SetSimulation (0, simulation, xOverY, viewNumber);
	viewNumber = ViewNumber::Enum (viewNumber + 1);
    }
}


QColor Settings::GetHighlightColor (
    ViewNumber::Enum viewNumber, HighlightNumber::Enum highlight) const
{
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	GetViewSettings (viewNumber).GetColorBarModel ();
    if (colorBarModel)
	return colorBarModel->GetHighlightColor (highlight);
    else
    {
	if (highlight == HighlightNumber::H0)
	    return Qt::black;
	else
	    return Qt::red;
    }
}

void Settings::SetViewNumber (ViewNumber::Enum viewNumber)
{
    m_viewNumber = viewNumber;
    Q_EMIT ViewChanged ();
}

void Settings::LinkedTimeBegin ()
{
    checkLinkedTimesValid ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t linkedTimeBegin = GetCurrentTime (viewNumber);
    size_t linkedTimeEnd = vs.GetLinkedTimeEnd ();
    checkLinkedTimesValid (linkedTimeBegin, linkedTimeEnd);
    vs.SetLinkedTimeBegin (linkedTimeBegin);
    Q_EMIT ViewChanged ();
}

void Settings::LinkedTimeEnd ()
{
    checkLinkedTimesValid ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t linkedTimeBegin = vs.GetLinkedTimeBegin ();
    size_t linkedTimeEnd = GetCurrentTime (viewNumber);
    checkLinkedTimesValid (linkedTimeBegin, linkedTimeEnd);
    vs.SetLinkedTimeEnd (linkedTimeEnd);
    Q_EMIT ViewChanged ();
}


void Settings::checkLinkedTimesValid (size_t timeBegin, size_t timeEnd) const
{
    if (timeBegin > timeEnd)
    {
	ostringstream ostr;
	ostr << "Error: timeBegin: " << timeBegin 
	     << " smaller than timeEnd: " << timeEnd;
	ThrowException (ostr.str ());
    }
}


void Settings::checkLinkedTimesValid () const
{
    if (GetTimeLinkage () == TimeLinkage::LINKED)
    {
	ThrowException ("You need to be in Settings > Show > "
			"View > Time linkage > Independent mode");
    }
}

size_t Settings::GetCurrentTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetCurrentTime ();
}

void Settings::SetTimeLinkage (TimeLinkage::Enum timeLinkage)
{
    m_timeLinkage = timeLinkage;
    SetCurrentTime (GetCurrentTime ());
    Q_EMIT ViewChanged ();
}

void Settings::SetCurrentTime (size_t currentTime, bool setLastStep)
{
    switch (GetTimeLinkage ()) 
    {
    case TimeLinkage::INDEPENDENT:
    {
	ViewNumber::Enum viewNumber = GetViewNumber ();
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetCurrentTime (currentTime, viewNumber);
	break;
    }
    case TimeLinkage::LINKED:
	m_linkedTime = currentTime;
	for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    ViewSettings& vs = GetViewSettings (viewNumber);
	    float multiplier = LinkedTimeStepStretch (viewNumber);
	    size_t timeSteps = GetTimeSteps (viewNumber);
	    size_t time = floor (m_linkedTime / multiplier);
	    if (time < timeSteps)
		vs.SetCurrentTime (time, viewNumber);
	    else if (setLastStep)
		vs.SetCurrentTime (timeSteps - 1, viewNumber);
	}
	break;
    }
}

void Settings::SetSimulation (int i, const Simulation& simulation, float xOverY,
			      ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    int rotation2D = simulation.GetRotation2D ();
    size_t reflexionAxis = simulation.GetReflectionAxis ();
    G3D::Vector3 center = CalculateViewingVolume (
	viewNumber, simulation, xOverY,
	ViewingVolumeOperation::DONT_ENCLOSE2D).center ();
    vs.SetSimulationIndex (i);
    vs.SetAxesOrder (
	simulation.Is2D () ? 
	(rotation2D == 0 ? AxesOrder::TWO_D :
	 (rotation2D == 90 ? AxesOrder::TWO_D_ROTATE_LEFT90 : 
	  ((reflexionAxis == 1) ? 
	   AxesOrder::TWO_D_ROTATE_RIGHT90_REFLECTION :
	   AxesOrder::TWO_D_ROTATE_RIGHT90))): AxesOrder::THREE_D);
    vs.SetT1sShiftLower (simulation.GetT1sShiftLower ());
    vs.AverageSetTimeWindow (simulation.GetTimeSteps ());
    vs.GetT1sPDE ().AverageSetTimeWindow (simulation.GetT1sTimeSteps ());
    vs.SetScaleCenter (center.xy ());
    vs.SetRotationCenter (center);
}

bool Settings::IsMissingPropertyShown (BodyScalar::Enum bodyProperty) const
{
    switch (bodyProperty)
    {
    case BodyScalar::PRESSURE:
	return m_missingPressureShown;
    case BodyScalar::TARGET_VOLUME:
    case BodyScalar::ACTUAL_VOLUME:
	return m_missingVolumeShown;
    case BodyScalar::VELOCITY_X:
    case BodyScalar::VELOCITY_Y:
    case BodyScalar::VELOCITY_MAGNITUDE:
	return m_objectVelocityShown;
    default:
	return true;
    }
}

QColor Settings::GetCenterPathContextColor () const
{
    QColor returnColor (Qt::black);
    returnColor.setAlphaF (GetContextAlpha ());
    return returnColor;
}

float Settings::LinkedTimeStepStretch (ViewNumber::Enum viewNumber) const
{
    return LinkedTimeStepStretch (LinkedTimeMaxInterval ().first, viewNumber);
}

float Settings::LinkedTimeStepStretch (size_t max,
				       ViewNumber::Enum viewNumber) const
{
    return static_cast<float> (max) / 
	GetViewSettings (viewNumber).GetLinkedTimeInterval ();
}

pair<size_t, ViewNumber::Enum> Settings::LinkedTimeMaxInterval () const
{
    pair<size_t, ViewNumber::Enum> max (0, ViewNumber::COUNT);
    for (size_t i = 0; i < ViewCount::GetCount (GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	size_t interval = vs.GetLinkedTimeInterval ();
	if (max.first < interval)
	{
	    max.first = interval;
	    max.second = viewNumber;
	}
    }
    return max;
}

float Settings::GetXOverY (float xOverY, ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    float v[] = { 
	xOverY, xOverY,     // ONE
	xOverY/2, 2*xOverY, // TWO (HORIZONTAL, VERTICAL)
	xOverY/3, 3*xOverY, // THREE (HORIZONTAL, VERTICAL)
	xOverY, xOverY      // FOUR
    };
    return v[GetViewCount () * 2 + GetViewLayout ()];
}

G3D::AABox Settings::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, const Simulation& simulation, 
    float xOverYWindow, ViewingVolumeOperation::Enum enclose) const
{    
    const ViewSettings& vs = GetViewSettings (viewNumber);
    float xOverY = GetXOverY (xOverYWindow, viewNumber);
    G3D::AABox bb = simulation.GetBoundingBox ();
    G3D::AABox vv = AdjustXOverYRatio (EncloseRotation (bb), xOverY);
    if (! simulation.Is2D ())
	// ExtendAlongZFor3D is used for 3D, 
	// so that you keep the 3D objects outside the camera
	vv = ExtendAlongZFor3D (vv, vs.GetScaleRatio ());
    if (enclose == ViewingVolumeOperation::ENCLOSE2D)
	vv = EncloseRotation2D (vv);
    return vv;
}
