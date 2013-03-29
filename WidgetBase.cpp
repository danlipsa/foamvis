/**
 * @file   WidgetBase.cpp
 * @author Dan R. Lipsa
 * @date 23 August 2012
 * 
 * Definitions for the widget for displaying foam bubbles
 */

#include "BodySelector.h"
#include "ColorBarModel.h"
#include "Debug.h"
#include "Settings.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"
#include "WidgetBase.h"



// Methods
// ======================================================================

WidgetBase::WidgetBase (QWidget* widget,
                        IsViewType isView,
                        GetViewCountType getViewCount) :
    m_averageCache (0),
    m_widget (widget), 
    m_isView (isView),
    m_getViewCount (getViewCount)
{
    initCopy (m_actionCopySelection, m_signalMapperCopySelection);
    initCopy (m_actionCopyTransformation, m_signalMapperCopyTransformation);

    m_actionResetTransformAll = 
        boost::make_shared<QAction> (tr("&All"), m_widget);
    m_actionResetTransformAll->setStatusTip(tr("Reset transform all"));

    m_actionResetTransformFocus = boost::make_shared<QAction> (
	tr("&Focus"), m_widget);
    m_actionResetTransformFocus->setStatusTip(tr("Reset transform focus"));
    
    m_actionColorMapEdit.reset (
	new QAction (tr("&Edit color map"), m_widget));
    m_actionColorMapEdit->setStatusTip(tr("Edit color map"));
    m_actionColorMapClampClear.reset (
	new QAction (tr("&Clamp clear"), m_widget));
    m_actionColorMapClampClear->setStatusTip(tr("Clamp clear"));
    initCopy (m_actionColorMapCopy, m_signalMapperColorMapCopy);

    m_actionOverlayMapEdit.reset (
	new QAction (tr("&Edit overlay map"), m_widget));
    m_actionOverlayMapEdit->setStatusTip(tr("Edit overlay map"));
   
    m_actionOverlayMapClampClear.reset (
	new QAction (tr("&Clamp clear"), m_widget));
    m_actionOverlayMapClampClear->setStatusTip(tr("Clamp clear"));

    m_actionOverlayMapCopyVelocityMagnitude.reset (
	new QAction (tr("&Copy velocity magnitude"), m_widget));
    m_actionOverlayMapCopyVelocityMagnitude->setStatusTip(
        tr("Copy velocity magnitude"));
}


QString WidgetBase::tr (const char * sourceText, 
                        const char * disambiguation, int n)
{
    return m_widget->tr (sourceText, disambiguation, n);
}


void WidgetBase::Init (
    boost::shared_ptr<Settings> settings,
    boost::shared_ptr<const SimulationGroup> simulationGroup, 
    AverageCaches* averageCache)
{
    SetSettings (settings);
    SetSimulationGroup (simulationGroup);
    m_averageCache = averageCache;
}

void WidgetBase::ForAllViews (boost::function <void (ViewNumber::Enum)> f)
{
    for (size_t i = 0; i < GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (CALL_MEMBER (*this, m_isView)(viewNumber))
	    f (viewNumber);
    }
}

void WidgetBase::ForAllHiddenViews (boost::function <void (ViewNumber::Enum)> f)
{
    for (int i = GetViewCount (); i < ViewNumber::COUNT; ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (CALL_MEMBER (*this, m_isView)(viewNumber))
	    f (viewNumber);
    }
}


G3D::AABox WidgetBase::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, const Simulation& simulation,
    ViewingVolumeOperation::Enum enclose) const
{
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = 
	CALL_MEMBER (*this, m_getViewCount) (&mapping);
    G3D::AABox vv = GetSettings ().CalculateViewingVolume (
	mapping[viewNumber], viewCount, simulation, 
	m_widget->width (), m_widget->height (), enclose);
    return vv;
}

G3D::Rect2D WidgetBase::GetViewRect (ViewNumber::Enum viewNumber) const
{
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = 
        CALL_MEMBER (*this, m_getViewCount) (&mapping);
    return GetSettings ().GetViewRect (
	m_widget->width (), m_widget->height (), mapping[viewNumber], viewCount);
}

void WidgetBase::contextMenuEventColorMap (QMenu* menu) const
{
    menu->addAction (m_actionColorMapClampClear.get ());
    addCopyCompatibleMenu (menu, "Copy", &m_actionColorMapCopy[0], 
                           &WidgetBase::IsColorBarCopyCompatible);
    menu->addAction (m_actionColorMapEdit.get ());
}

void WidgetBase::contextMenuEventOverlayMap (QMenu* menu) const
{
    menu->addAction (m_actionOverlayMapClampClear.get ());
    if (GetViewSettings ().GetBodyOrFaceScalar () == 
        BodyScalar::VELOCITY_MAGNITUDE)
        menu->addAction (m_actionOverlayMapCopyVelocityMagnitude.get ());
    menu->addAction (m_actionOverlayMapEdit.get ());
}


void WidgetBase::contextMenuEvent (QContextMenuEvent *event)
{
    QMenu menu (m_widget);
    ViewNumber::Enum viewNumber = GetViewNumber ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    QPoint contextMenuPosWindow = event->pos ();
    G3D::Rect2D barRect = 
        GetSettings ().GetColorBarRectWithLabels (
            viewNumber, GetViewRect ());
    G3D::Rect2D overlayBarRect = 
	GetSettings ().GetOverlayBarRectWithLabels (
            viewNumber, GetViewRect ());
    if (vs.IsScalarShown () && barRect.contains (
            QtToOpenGl (contextMenuPosWindow, m_widget->height ())))
	contextMenuEventColorMap (&menu);
    else if (vs.IsVelocityShown () &&
             overlayBarRect.contains (
		 QtToOpenGl (contextMenuPosWindow, m_widget->height ())))
	contextMenuEventOverlayMap (&menu);
    else
	contextMenuEventView (&menu);
    menu.exec (event->globalPos());
}

void WidgetBase::setView (const G3D::Vector2& clickedPoint)
{
    ForAllViews (boost::bind (&WidgetBase::setView, this, _1, clickedPoint));
}
void WidgetBase::setView (ViewNumber::Enum viewNumber, 
			  const G3D::Vector2& clickedPoint)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    if (viewRect.contains (clickedPoint))
	GetSettingsPtr ()->SetViewNumber (viewNumber);
}

void WidgetBase::addCopyMenu (
    QMenu* menu, const char* nameOp, 
    const boost::shared_ptr<QAction>* actionCopyOp) const
{
    if (GetViewCount () > 1)
    {
	QMenu* menuOp = menu->addMenu (nameOp);
	for (size_t i = 0; i < GetViewCount (); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    if (viewNumber == GetViewNumber ())
		continue;
	    menuOp->addAction (actionCopyOp[i].get ());
	}
    }
}

void WidgetBase::addCopyCompatibleMenu (
    QMenu* menu, const char* nameOp, 
    const boost::shared_ptr<QAction>* actionCopyOp, 
    IsCopyCompatibleType isCopyCompatible) const
{
    size_t viewCount = GetSettings ().GetViewCount ();
    bool actions = false;
    QMenu* menuOp = menu->addMenu (nameOp);
    if (viewCount > 1)
    {
        ViewNumber::Enum currentViewNumber = GetViewNumber ();
	for (size_t i = 0; i < viewCount; ++i)
	{
	    ViewNumber::Enum otherViewNumber = ViewNumber::Enum (i);
	    if (CALL_MEMBER (*this, isCopyCompatible) (
                    currentViewNumber, otherViewNumber))
            {
                menuOp->addAction (actionCopyOp[i].get ());
                actions = true;
            }
	}
    }
    if (! actions)
	menuOp->setDisabled (true);    
}

bool WidgetBase::IsColorBarCopyCompatible (
    ViewNumber::Enum vn, ViewNumber::Enum otherVn) const
{
    ColorBarType::Enum currentColorBarType = 
        GetSettings ().GetColorBarType (vn);
    const ViewSettings& vs = GetViewSettings (vn);
    const ViewSettings& otherVs = GetViewSettings (otherVn);
    size_t currentProperty = vs.GetBodyOrFaceScalar ();
    return otherVn != vn &&
        
        currentColorBarType == GetSettings ().GetColorBarType (otherVn) &&
        
        ((currentColorBarType == ColorBarType::T1S_KDE) 
         ||
         currentProperty == otherVs.GetBodyOrFaceScalar ());
}


bool WidgetBase::IsSelectionCopyCompatible (
    ViewNumber::Enum vn, ViewNumber::Enum otherVn) const
{
    const ViewSettings& vs = GetViewSettings (vn);
    const ViewSettings& otherVs = GetViewSettings (otherVn);
    return otherVn != vn && 
        vs.GetSimulationIndex () == otherVs.GetSimulationIndex () &&
        (vs.GetBodyOrFaceScalar () == otherVs.GetBodyOrFaceScalar () ||
         otherVs.GetBodySelector ().GetType () == BodySelectorType::ID || 
         otherVs.GetBodySelector ().GetType () == BodySelectorType::ALL);
}


void WidgetBase::initCopy (
    boost::array<boost::shared_ptr<QAction>, ViewNumber::COUNT>& actionCopyOp,
    boost::shared_ptr<QSignalMapper>& signalMapperCopyOp)
{
    signalMapperCopyOp.reset (new QSignalMapper (m_widget));
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	ostringstream ostr;
	ostr << "View " << i;
	QString text (ostr.str ().c_str ());
	actionCopyOp[i] = boost::make_shared<QAction> (text, m_widget);
	actionCopyOp[i]->setStatusTip(text);
	m_widget->connect (actionCopyOp[i].get (), 
                           SIGNAL(triggered()),
                           signalMapperCopyOp.get (), 
                           SLOT(map ()));
	signalMapperCopyOp->setMapping (
	    actionCopyOp[i].get (), i);
    }
}


const Foam& WidgetBase::GetFoam (ViewNumber::Enum viewNumber, 
                                 size_t timeStep) const
{
    return GetSimulation (viewNumber).GetFoam (timeStep);
}

G3D::Matrix3 WidgetBase::GetRotationForAxesOrder (ViewNumber::Enum viewNumber, 
                                                  size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    const Foam& foam = simulation.GetFoam (timeStep);
    return vs.GetRotationForAxesOrder (foam);
}

void WidgetBase::ResetTransformFocus ()
{
    vector<ViewNumber::Enum> vn = GetSettings ().GetTwoHalvesViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetRotation (G3D::Matrix3::identity ());
	vs.SetScaleRatio (1);
	vs.SetTranslation (G3D::Vector3::zero ());
    }
}

float WidgetBase::GetDeformationSizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    float gridCellLength = GetBubbleDiameter (viewNumber);
    return gridCellLength / (2 * simulation.GetMaxDeformationEigenValue ());
}

float WidgetBase::GetVelocitySizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    float bubbleDiameter = GetBubbleDiameter (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    float velocityRange = simulation.GetMax (BodyScalar::VELOCITY_MAGNITUDE);
    // min velocity is 0.
    return bubbleDiameter / velocityRange;
}
