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


Settings::Settings (const Simulation& simulation, float xOverY, 
		    bool t1sShiftLower) :
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
    m_centerPathLineUsed (false),
    m_splitHalfView (false)
{
    initViewSettings (simulation, xOverY, t1sShiftLower);
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

void Settings::initViewSettings (
    const Simulation& simulation, float xOverY, bool t1sShiftLower)
{
    ViewNumber::Enum viewNumber (ViewNumber::VIEW0);
    BOOST_FOREACH (boost::shared_ptr<ViewSettings>& vs, m_viewSettings)
    {
	vs = boost::make_shared <ViewSettings> ();
	vs->SetViewType (ViewType::FACES);
	G3D::Vector3 center = CalculateViewingVolume (
	    viewNumber, simulation, xOverY,
	    ViewingVolumeOperation::DONT_ENCLOSE2D).center ();
	vs->SetSimulation (0, simulation, center, t1sShiftLower);
	if (simulation.Is3D ())
	    vs->SetLightEnabled (LightNumber::LIGHT0, true);

	boost::array<GLfloat, 4> ambientLight = {{0, 0, 0, 1}};
	boost::array<GLfloat, 4> diffuseLight = {{1, 1, 1, 1}};
	boost::array<GLfloat, 4> specularLight = {{0, 0, 0, 1}};
	// light colors
	for (size_t light = 0; light < LightNumber::COUNT; ++light)
	{
	    vs->SetLight (LightNumber::Enum (light), 
			  LightType::AMBIENT, ambientLight);
	    vs->SetLight (LightNumber::Enum (light), 
			  LightType::DIFFUSE, diffuseLight);
	    vs->SetLight (LightNumber::Enum (light), 
			  LightType::SPECULAR, specularLight);
	}
	vs->CalculateCameraDistance (
	    CalculateCenteredViewingVolume (
		viewNumber, simulation, xOverY,
		ViewingVolumeOperation::DONT_ENCLOSE2D));
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
    ViewNumber::Enum prevViewNumber = m_viewNumber;
    m_viewNumber = viewNumber;
    Q_EMIT ViewChanged (prevViewNumber);
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
    Q_EMIT ViewChanged (viewNumber);
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
    Q_EMIT ViewChanged (viewNumber);
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
    Q_EMIT ViewChanged (GetViewNumber ());
}

void Settings::SetCurrentTime (
    size_t currentTime, 
    boost::array<int, ViewNumber::COUNT>* d, bool setLastStep)
{
    boost::array<int, ViewNumber::COUNT> direction;
    fill (direction.begin (), direction.end (), 0);
    switch (GetTimeLinkage ()) 
    {
    case TimeLinkage::INDEPENDENT:
    {
	ViewNumber::Enum viewNumber = GetViewNumber ();
	ViewSettings& vs = GetViewSettings (viewNumber);
	direction[viewNumber] = vs.SetCurrentTime (currentTime);
	break;
    }
    case TimeLinkage::LINKED:
	m_linkedTime = currentTime;
	for (int i = 0; i < m_viewCount; ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    ViewSettings& vs = GetViewSettings (viewNumber);
	    float multiplier = LinkedTimeStepStretch (viewNumber);
	    size_t timeSteps = vs.GetTimeSteps ();
	    size_t time = floor (m_linkedTime / multiplier);
	    if (time < timeSteps)
		direction[viewNumber] = vs.SetCurrentTime (time);
	    else if (setLastStep)
		direction[viewNumber] = vs.SetCurrentTime (timeSteps - 1);
	}
	break;
    }
    if (d)
	copy (direction.begin (), direction.end (), d->begin ());
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
    for (int i = 0; i < GetViewCount (); ++i)
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

G3D::AABox Settings::CalculateCenteredViewingVolume (
    ViewNumber::Enum viewNumber, const Simulation& simulation, 
    float xOverYWindow, ViewingVolumeOperation::Enum enclose) const
{
    G3D::AABox box = CalculateViewingVolume (viewNumber, simulation,
					     xOverYWindow, enclose);
    return box - box.center ();
}

G3D::AABox Settings::CalculateEyeViewingVolume (
    ViewNumber::Enum viewNumber, const Simulation& simulation, 
    float xOverYWindow, ViewingVolumeOperation::Enum enclose) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::AABox vv = CalculateViewingVolume (viewNumber, simulation, 
					    xOverYWindow, enclose);
    vv = vv - vv.center ();
    G3D::Vector3 translation (vs.GetCameraDistance () * G3D::Vector3::unitZ ());
    G3D::AABox result = vv - translation;
    return result;
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

float Settings::GetXOverY (float xOverY, ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    float v[] = { 
	xOverY, xOverY,     // ONE
	xOverY/2, 2*xOverY, // TWO (HORIZONTAL, VERTICAL)
	xOverY/3, 3*xOverY, // THREE (HORIZONTAL, VERTICAL)
	xOverY, xOverY      // FOUR
    };
    return v[(GetViewCount () - 1) * 2 + GetViewLayout ()];
}

bool Settings::IsVtkView (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = 
	GetViewSettings (ViewNumber::FromSizeT (viewNumber));
    return DATA_PROPERTIES.Is3D () && vs.GetViewType () == ViewType::AVERAGE;
}

ViewCount::Enum Settings::GetVtkCount (vector<ViewNumber::Enum>* mapping) const
{
    ViewCount::Enum viewCount = GetViewCount ();
    mapping->resize (viewCount);
    fill (mapping->begin (), mapping->end (), ViewNumber::COUNT);
    int vtkCount = 0;
    for (int i = 0; i < viewCount; ++i)
	if (IsVtkView (ViewNumber::FromSizeT (i)))
	{
	    (*mapping)[i] = ViewNumber::FromSizeT (vtkCount);
	    vtkCount++;
	}
    return ViewCount::FromSizeT (vtkCount);
}

ViewCount::Enum Settings::GetGlCount (vector<ViewNumber::Enum>* mapping) const
{
    vector<ViewNumber::Enum> m;
    if (mapping == 0)
	mapping = &m;
    ViewCount::Enum viewCount = GetVtkCount (mapping);
    // complement the result
    viewCount = ViewCount::FromSizeT (GetViewCount () - viewCount);
    int glViewNumber = 0;
    for (size_t i = 0; i < mapping->size (); ++i)
	if ((*mapping)[i] == ViewNumber::COUNT)
	    (*mapping)[i] = ViewNumber::FromSizeT (glViewNumber++);
	else
	    (*mapping)[i] = ViewNumber::COUNT;
    RuntimeAssert (glViewNumber == viewCount, 
		   "GetGLCount: counts should be equal:", 
		   viewCount, glViewNumber);
    return viewCount;
}

ViewType::Enum Settings::SetConnectedViewType (ViewType::Enum viewType)
{
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    ViewType::Enum oldViewType = GetViewSettings (vn[0]).GetViewType ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetViewType (viewType);
    }
    return oldViewType;
}


G3D::Rect2D Settings::GetViewRect (
    float w, float h,
    ViewNumber::Enum viewNumber, ViewCount::Enum viewCount) const
{
    using G3D::Rect2D;
    switch (viewCount)
    {
    case ViewCount::ZERO:
    case ViewCount::COUNT:
	RuntimeAssert (false, "Invalid view count:", viewCount);
	return G3D::Rect2D ();
    case ViewCount::ONE:
	return Rect2D::xywh (0, 0, w, h);
    case ViewCount::TWO:
    {
	RuntimeAssert (viewNumber < 2, "Invalid view: ", viewNumber);
	Rect2D v[][2] = {
	    // 0 | 1
	    // horizontal layout
	    {Rect2D::xywh (0, 0, w/2, h), Rect2D::xywh (w/2, 0, w/2, h)},
	    // 0
	    // -
	    // 1
	    // vertical layout
	    {Rect2D::xywh (0, h/2, w, h/2), Rect2D::xywh (0, 0, w, h/2)}
	};
	return v[GetViewLayout ()][viewNumber];
    }
    case ViewCount::THREE:
    {
	RuntimeAssert (viewNumber < 3, "Invalid view: ", viewNumber);
	Rect2D v[][3] = {
	    // 0 | 1 | 3
	    // horizontal layout
	    {Rect2D::xywh (0, 0, w/3, h),
	     Rect2D::xywh (w/3, 0, w/3, h),
	     Rect2D::xywh (2*w/3, 0, w/3, h)},
	    // 0
	    // -
	    // 1
	    // -
	    // 3
	    // vertical layout
	    {Rect2D::xywh (0, 2*h/3, w, h/3),
	     Rect2D::xywh (0, h/3, w, h/3), 
	     Rect2D::xywh (0, 0, w, h/3)}
	};
	return v[GetViewLayout ()][viewNumber];
    }
    case ViewCount::FOUR:
    {
	//  0 | 1
	//  -----
	//  2 | 3
	RuntimeAssert (viewNumber < 4, "Invalid view: ", viewNumber);
	Rect2D v[] = {
	    Rect2D::xywh (0, h/2, w/2, h/2), Rect2D::xywh (w/2, h/2, w/2, h/2),
	    Rect2D::xywh (0, 0, w/2, h/2), Rect2D::xywh (w/2, 0, w/2, h/2)
	};
	return v[viewNumber];
    }
    }
    return G3D::Rect2D ();
}



G3D::Rect2D Settings::GetViewColorBarRect (const G3D::Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
	viewRect.x0 () + 15, viewRect.y0 () + 15,
	10, max (viewRect.height () / 4, 50.0f));
}

G3D::Rect2D Settings::GetViewOverlayBarRect (const G3D::Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
	viewRect.x0 () + 15 + 10 + 5, viewRect.y0 () + 15,
	10, max (viewRect.height () / 4, 50.0f));
}

vector<ViewNumber::Enum> Settings::GetConnectedViewNumbers (
    ViewNumber::Enum viewNumber) const
{
    if (m_splitHalfView)
    {
	vector<ViewNumber::Enum> vn(2);
	vn[0] = ViewNumber::VIEW0;
	vn[1] = ViewNumber::VIEW1;
	return vn;
    }
    else
    {
	vector<ViewNumber::Enum> vn(1);
	vn[0] = viewNumber;
	return vn;
    }
}

void Settings::SetSplitHalfView (bool reflectedHalfView, 
				     const Simulation& simulation, float xOverY)
{
    m_splitHalfView = reflectedHalfView;
    setScaleCenter (ViewNumber::VIEW0, simulation, xOverY);
    setScaleCenter (ViewNumber::VIEW1, simulation, xOverY);
}

void Settings::setScaleCenter (
    ViewNumber::Enum viewNumber, const Simulation& simulation, float xOverY)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D rect = 
	toRect2D (CalculateViewingVolume (
		      viewNumber, simulation, xOverY,
		      ViewingVolumeOperation::DONT_ENCLOSE2D));
    G3D::Vector2 newCenter = CalculateScaleCenter (viewNumber, rect);
    vs.SetScaleCenter (newCenter);
}

G3D::Vector2 Settings::CalculateScaleCenter (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& rect) const
{
    if (! IsSplitHalfView ())
	return rect.center ();
    else if (viewNumber == ViewNumber::VIEW0)
	return (rect.x0y0 () + rect.x1y0 ()) / 2;
    else
	return (rect.x0y1 () + rect.x1y1 ()) / 2;
}
