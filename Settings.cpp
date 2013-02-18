/**
 * @file   Settings.cpp
 * @author Dan R. Lipsa
 * @date 17 July 2012
 *
 * Definitions for the view settings
 */

#include "ColorBarModel.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "MainWindow.h"
#include "Settings.h"
#include "Simulation.h"
#include "T1sKDE.h"
#include "Utils.h"
#include "ViewSettings.h"
#include "WidgetGl.h"


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

class StringsRect
{
public:
    StringsRect (const QFontMetrics& fm) :
        m_fm (fm), m_width (0)
    {
    }
    
    void StoreMax (const string& s)
    {
        QRect br = m_fm.tightBoundingRect (s.c_str ());
        if (m_width < br.width ())
            m_width = br.width ();
        if (m_height < br.height ())
            m_height = br.height ();
    }
    float GetMaxWidth () const
    {
        return m_width;
    }
    float GetMaxHeight () const
    {
        return m_height;
    }

private:
    const QFontMetrics& m_fm;
    float m_width;
    float m_height;
};


// Methods
// ======================================================================
const size_t Settings::QUADRIC_SLICES = 8;
const size_t Settings::QUADRIC_STACKS = 1;


Settings::Settings (const SimulationGroup& simulationGroup, float w, float h) :
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
    m_linkedTimeWindow (0),
    m_viewCount (ViewCount::ONE),
    m_viewLayout (ViewLayout::HORIZONTAL),
    m_missingPressureShown (true),
    m_missingVolumeShown (true),
    m_objectVelocityShown (false),
    m_centerPathTubeUsed (true),
    m_centerPathLineUsed (false),
    m_splitHalfView (false),
    m_titleShown (true),
    m_viewFocusShown (true),
    m_barLarge (false),
    m_velocityFieldSaved (false),
    m_barLabelsShown (true),
    m_interactionMode (InteractionMode::ROTATE)
{
    initAllViewsSettings (simulationGroup, w, h);
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

size_t Settings::initViewSettings (ViewNumber::Enum viewNumber,
    const SimulationGroup& simulationGroup, float w, float h)
{
    ViewCount::Enum viewCount = ViewCount::FromSizeT (m_viewSettings.size ());
    boost::shared_ptr<ViewSettings>& vs = m_viewSettings[viewNumber];
    size_t simulationIndex = 
        (static_cast<size_t>(viewNumber) < simulationGroup.size ()) ? 
        viewNumber : 0;
    const Simulation& simulation = 
        simulationGroup.GetSimulation (simulationIndex);
    vs = boost::make_shared <ViewSettings> ();
    connect (vs.get (), SIGNAL (SelectionChanged ()),
             m_signalMapperSelectionChanged.get (), SLOT (map ()));
    m_signalMapperSelectionChanged->setMapping (vs.get (), viewNumber);
    vs->SetViewType (ViewType::FACES);
    G3D::Vector3 center = CalculateViewingVolume (
        viewNumber, viewCount, simulation, w, h,
        ViewingVolumeOperation::DONT_ENCLOSE2D).center ();
    vs->SetSimulation (simulationIndex, simulation, center);
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
            viewNumber, viewCount, simulation, w, h,
            ViewingVolumeOperation::DONT_ENCLOSE2D));
    return simulation.GetTimeSteps ();
}



void Settings::initAllViewsSettings (
    const SimulationGroup& simulationGroup, float w, float h)
{
    m_signalMapperSelectionChanged.reset (new QSignalMapper (this));
    size_t maxTimeSteps = 0;
    for (size_t i = 0; i < m_viewSettings.size (); ++i)
    {
        ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
        size_t timeSteps = initViewSettings (viewNumber, simulationGroup, w, h);
        maxTimeSteps = max (maxTimeSteps, timeSteps);
    }
    connect (
        m_signalMapperSelectionChanged.get (),
        SIGNAL (mapped (int)),
        this, 
        SLOT (selectionChanged (int)));
    SetLinkedTimeWindow (maxTimeSteps);
}

void Settings::selectionChanged (int viewNumber)
{
    Q_EMIT SelectionChanged (ViewNumber::FromSizeT (viewNumber));
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


size_t Settings::GetViewTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetTime ();
}

void Settings::SetTimeLinkage (TimeLinkage::Enum timeLinkage)
{
    m_timeLinkage = timeLinkage;
    SetTime (GetViewTime ());
    Q_EMIT ViewChanged (GetViewNumber ());
}

void Settings::SetTime (
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
	direction[viewNumber] = vs.SetTime (currentTime);
	break;
    }
    case TimeLinkage::LINKED:
	m_linkedTime = currentTime;
	for (int i = 0; i < m_viewCount; ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
            direction[viewNumber] = 
                setCurrentTime (viewNumber, m_linkedTime, setLastStep);
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

G3D::AABox Settings::CalculateCenteredViewingVolume (
    ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
    const Simulation& simulation, 
    float w, float h, ViewingVolumeOperation::Enum enclose) const
{
    G3D::AABox box = CalculateViewingVolume (viewNumber, viewCount, simulation,
					     w, h, enclose);
    return box - box.center ();
}

G3D::AABox Settings::CalculateEyeViewingVolume (
    ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
    const Simulation& simulation, 
    float w, float h, ViewingVolumeOperation::Enum enclose) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::AABox vv = CalculateViewingVolume (viewNumber, viewCount, simulation, 
					    w, h, enclose);
    vv = vv - vv.center ();
    G3D::Vector3 translation (vs.GetCameraDistance () * G3D::Vector3::unitZ ());
    G3D::AABox result = vv - translation;
    return result;
}



G3D::AABox Settings::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
    const Simulation& simulation, 
    float w, float h, ViewingVolumeOperation::Enum enclose) const
{    
    const ViewSettings& vs = GetViewSettings (viewNumber);
    float xOverY = getXOverY (w, h, viewNumber, viewCount);
    G3D::AABox bb = simulation.GetBoundingBox ();
    G3D::AABox vv = AdjustXOverYRatio (EncloseRotation (bb), xOverY);
    if (! simulation.Is2D ())
	// ExtendAlongZFor3D is used for 3D, 
	// so that you keep the 3D objects outside the camera
	vv = ExtendAlongZFor3D (vv, vs.GetScaleRatio ());
    if (enclose == ViewingVolumeOperation::ENCLOSE2D)
	vv = EncloseRotation2D (vv);
    RuntimeAssert (IsFuzzyZero (bb.center () - vv.center ()), 
                   "Simulation center different than viewing volume center: ", 
                   bb.center (), vv.center ());
    return vv;
}


ViewType::Enum Settings::SetTwoHalvesViewType (ViewType::Enum viewType)
{
    vector<ViewNumber::Enum> vn = GetTwoHalvesViewNumbers ();
    ViewType::Enum oldViewType = GetViewSettings (vn[0]).GetViewType ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetViewType (viewType);
    }
    return oldViewType;
}


float Settings::getXOverY (float w, float h, ViewNumber::Enum viewNumber, 
			   ViewCount::Enum viewCount) const
{
    G3D::Rect2D rect = GetViewRect (w, h, viewNumber, viewCount);
    return rect.width () / rect.height ();
}



G3D::Rect2D Settings::GetViewRect (
    float w, float h,
    ViewNumber::Enum viewNumber, size_t viewCount) const
{
    using G3D::Rect2D;
    RuntimeAssert (viewNumber != ViewNumber::COUNT,
		   "Invalid view number ViewNumber::COUNT");
    switch (viewCount)
    {
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



G3D::Rect2D Settings::GetViewColorBarRect (const G3D::Rect2D& viewRect) const
{
    const float d = 15.0;
    float barHeight = 
        m_barLarge ? (viewRect.height () - 2 * d) : 
        max (viewRect.height () / 4, 50.0f);
    return G3D::Rect2D::xywh (viewRect.x0 () + d, viewRect.y0 () + d,
                              10, barHeight);
}

G3D::Rect2D Settings::GetViewOverlayBarRect (const G3D::Rect2D& viewRect) const
{
    const float d = 15.0;
    float barHeight = 
        m_barLarge ? (viewRect.height () - 2 * d): 
        max (viewRect.height () / 4, 50.0f);
    return G3D::Rect2D::xywh (viewRect.x0 () + d + 10 + 5, viewRect.y0 () + d,
                              10, barHeight);
}

G3D::Vector2 Settings::GetBarLabelsSize (ViewNumber::Enum viewNumber) const
{
    const ColorBarModel& cbm = *GetViewSettings (viewNumber).GetColorBarModel ();
    //__ENABLE_LOGGING__;
    if (! BarLabelsShown ())
        return G3D::Vector2 (0, 0);
    QFont font;
    QFontMetrics fm (font);
    StringsRect sr(fm);
    const float distancePixels = 10;
    ostringstream ostr;
    QwtDoubleInterval interval = cbm.GetInterval ();
    ostr << scientific << setprecision (1);
    ostr << interval.minValue ();
    sr.StoreMax (ostr.str ());
    ostr.str ("");ostr << interval.maxValue ();
    sr.StoreMax (ostr.str ());
    if (cbm.IsClampedMin ())
    {
        ostr.str ("");ostr << cbm.GetClampMin ();
        sr.StoreMax (ostr.str ());
    }
    if (cbm.IsClampedMax ())
    {
        ostr.str ("");ostr << cbm.GetClampMax ();
        sr.StoreMax (ostr.str ());
    }
    return G3D::Vector2 (sr.GetMaxWidth () + distancePixels, sr.GetMaxHeight ());
}


G3D::Rect2D Settings::GetViewColorBarRectWithLabels (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect) const
{
    G3D::Rect2D rect = GetViewColorBarRect (viewRect);
    G3D::Vector2 s = GetBarLabelsSize (viewNumber);
    return G3D::Rect2D::xywh (rect.x0 (), rect.y0 () + s.y, 
                              rect.width () + s.x, rect.height () + s.y);
}

vector<ViewNumber::Enum> Settings::GetTwoHalvesViewNumbers (
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

void Settings::SetTwoHalvesView (bool reflectedHalfView, 
				 const Simulation& simulation, float w, float h)
{
    m_splitHalfView = reflectedHalfView;
    setScaleCenter (ViewNumber::VIEW0, simulation, w, h);
    setScaleCenter (ViewNumber::VIEW1, simulation, w, h);
}

void Settings::setScaleCenter (
    ViewNumber::Enum viewNumber, const Simulation& simulation, float w, float h)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D rect = 
	toRect2D (CalculateViewingVolume (
		      viewNumber, ViewCount::TWO, simulation, w, h,
		      ViewingVolumeOperation::DONT_ENCLOSE2D));
    G3D::Vector2 newCenter = CalculateScaleCenter (viewNumber, rect);
    vs.SetScaleCenter (newCenter);
}

G3D::Vector2 Settings::CalculateScaleCenter (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& rect) const
{
    if (! IsTwoHalvesView ())
	return rect.center ();
    else if (viewNumber == ViewNumber::VIEW0)
	return (rect.x0y0 () + rect.x1y0 ()) / 2;
    else
	return (rect.x0y1 () + rect.x1y1 ()) / 2;
}

ColorBarType::Enum Settings::GetColorBarType () const
{
    return GetColorBarType (GetViewNumber ());
}

ColorBarType::Enum Settings::GetColorBarType (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetColorBarType ();
}

template<typename T>
void Settings::SetOneOrTwoViews (T* t, void (T::*f) (ViewNumber::Enum))
{
    if (IsTwoHalvesView ())
    {
	CALL_MEMBER (*t, f) (ViewNumber::VIEW0);
	CALL_MEMBER (*t, f) (ViewNumber::VIEW1);
    }
    else
	CALL_MEMBER (*t, f) (GetViewNumber ());
}

void Settings::AddLinkedTimeEvent ()
{
    checkLinkedTimesValid ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t currentTime = GetViewTime (viewNumber);
    vs.AddLinkedTimeEvent (currentTime);
    Q_EMIT ViewChanged (viewNumber);
}

void Settings::ResetLinkedTimeEvents ()
{
    checkLinkedTimesValid ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.ResetLinkedTimeEvents ();
    Q_EMIT ViewChanged (viewNumber);
}

void Settings::checkLinkedTimesValid () const
{
    if (GetTimeLinkage () == TimeLinkage::LINKED)
    {
	ThrowException ("You need to be in Settings > Time > Independent");
    }
}

float Settings::GetLinkedTimeStretch (
    ViewNumber::Enum viewNumber, size_t eventIndex) const
{
    return static_cast<float> (GetLinkedTimeMaxInterval (eventIndex).first) / 
        GetViewSettings (viewNumber).GetLinkedTimeInterval (eventIndex);
}

pair<size_t, ViewNumber::Enum> Settings::GetLinkedTimeMaxInterval (
    size_t eventIndex) const
{
    RuntimeAssert (HasEqualNumberOfEvents (),
                   "You need to have an equal number of events in all views");
    pair<size_t, ViewNumber::Enum> max (0, ViewNumber::COUNT);
    for (size_t i = 0; i < GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	size_t interval = vs.GetLinkedTimeInterval (eventIndex);
	if (max.first < interval)
	{
	    max.first = interval;
	    max.second = viewNumber;
	}
    }
    return max;
}

vector<ViewNumber::Enum> Settings::GetLinkedTimeViewNumbers (
    ViewNumber::Enum viewNumber) const
{
    if (GetTimeLinkage () == TimeLinkage::LINKED)
    {
        size_t viewCount = GetViewCount ();
        vector<ViewNumber::Enum> vn (viewCount);
        for (size_t i = 0; i < viewCount; ++i)
            vn[i] = ViewNumber::FromSizeT (i);
        return vn;
    }
    else
    {
        vector<ViewNumber::Enum> vn (1);
        vn[0] = viewNumber;
        return vn;
    }
}

const vector<size_t>& Settings::GetLinkedTimeEvents (
    ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetLinkedTimeEvents ();
}

int Settings::calculateViewTime (
    ViewNumber::Enum viewNumber, int linkedTime) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    // search for the event interval where the linked time is, and compute
    // the view time
    int currentViewTime = 0;
    int currentLinkedTime = 0;
    size_t eventIndex = 0;
    while(eventIndex < GetLinkedTimeEvents (ViewNumber::VIEW0).size ())
    {
        pair<size_t, ViewNumber::Enum> p = GetLinkedTimeMaxInterval (eventIndex);
        if (static_cast<int>(currentLinkedTime + p.first - 1) > linkedTime)
            break;
        currentLinkedTime += (p.first - 1);
        currentViewTime += (vs.GetLinkedTimeInterval (eventIndex) - 1);
        ++eventIndex;
    }
    if (eventIndex == GetLinkedTimeEvents (ViewNumber::VIEW0).size ())
    {
        int linkedTimeAtEnd = linkedTime - currentLinkedTime;
        int viewTimeAtEnd = vs.GetTimeSteps () - currentViewTime - 1;
        currentViewTime += min (viewTimeAtEnd, linkedTimeAtEnd);
    }
    else
        currentViewTime += (linkedTime - currentLinkedTime) / 
            GetLinkedTimeStretch (viewNumber, eventIndex);
    return currentViewTime;
}

size_t Settings::GetTimeSteps (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetTimeSteps ();
}


int Settings::setCurrentTime (
    ViewNumber::Enum viewNumber, size_t linkedTime, bool setLastStep)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t viewTime = calculateViewTime (viewNumber, linkedTime);
    if (viewTime >= vs.GetTimeSteps ())
    {
        if (setLastStep)
            viewTime = vs.GetTimeSteps () - 1;
        else
            return 0;
    }
    return vs.SetTime (viewTime);
}

size_t Settings::GetLinkedTimeSteps () const
{
    size_t currentLinkedTime = 0;
    for (size_t eventIndex = 0; 
         eventIndex <= GetLinkedTimeEvents (ViewNumber::VIEW0).size ();
         ++eventIndex)
    {
        pair<size_t, ViewNumber::Enum> p = GetLinkedTimeMaxInterval (eventIndex);
        currentLinkedTime += p.first;
    }
    return currentLinkedTime;
}

bool Settings::HasEqualNumberOfEvents () const
{
    size_t numberOfEvents = GetLinkedTimeEvents (ViewNumber::VIEW0).size ();
    for (size_t i = 1; i < GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
        if (numberOfEvents != GetLinkedTimeEvents (viewNumber).size ())
            return false;
    }
    return true;
}

size_t Settings::GetLinkedTimeEventTime (size_t eventIndex) const
{
    size_t eventTime = -1;
    for (size_t i = 0; i <= eventIndex; ++i)
        eventTime += GetLinkedTimeMaxInterval (i).first;
    return eventTime;
}

size_t Settings::CalculateViewTimeWindow (
    ViewNumber::Enum viewNumber, size_t linkedTimeHigh) const
{
    int linkedTimeLow = linkedTimeHigh - m_linkedTimeWindow + 1;
    int viewHigh = calculateViewTime (viewNumber, linkedTimeHigh);
    int viewLow = calculateViewTime (viewNumber, linkedTimeLow);
    return  viewHigh - viewLow + 1;
}

void Settings::SetAverageTimeWindow (size_t timeSteps)
{
    SetLinkedTimeWindow (timeSteps);
    UpdateAverageTimeWindow ();
}

void Settings::UpdateAverageTimeWindow ()
{
    for (size_t i = 0; i < GetViewCount (); ++i)
    {
        ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
        GetViewSettings (viewNumber).SetTimeWindow (
            CalculateViewTimeWindow (viewNumber, GetLinkedTime ()));
    }
}


// Template instantiations
// ======================================================================
/// @cond
template
void Settings::SetOneOrTwoViews<MainWindow> (
    MainWindow* t, void (MainWindow::*f) (ViewNumber::Enum));
template
void Settings::SetOneOrTwoViews<WidgetGl> (
    WidgetGl* t, void (WidgetGl::*f) (ViewNumber::Enum));
/// @endcond
