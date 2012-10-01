/**
 * @file   WidgetBase.cpp
 * @author Dan R. Lipsa
 * @date 23 August 2012
 * 
 * Definitions for the widget for displaying foam bubbles
 */


#include "Debug.h"
#include "Settings.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"
#include "WidgetBase.h"


void WidgetBase::Init (
    boost::shared_ptr<Settings> settings,
    const SimulationGroup* simulationGroup, AverageCache* averageCache)
{
    m_settings = settings;
    m_simulationGroup = simulationGroup;
    m_averageCache = averageCache;
}

void WidgetBase::ForAllViews (boost::function <void (ViewNumber::Enum)> f)
{
    for (int i = 0; i < GetSettings ()->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (CALL_MEMBER (*GetSettings (), m_isView)(viewNumber))
	    f (viewNumber);
    }
}

void WidgetBase::ForAllHiddenViews (boost::function <void (ViewNumber::Enum)> f)
{
    for (int i = GetSettings ()->GetViewCount (); i < ViewNumber::COUNT; ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (CALL_MEMBER (*GetSettings (), m_isView)(viewNumber))
	    f (viewNumber);
    }
}


G3D::AABox WidgetBase::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, 
    ViewingVolumeOperation::Enum enclose) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = 
	CALL_MEMBER (*GetSettings (), m_getViewCount) (&mapping);
    G3D::AABox vv = GetSettings ()->CalculateViewingVolume (
	mapping[viewNumber], viewCount, simulation, 
	m_widget->width (), m_widget->height (), enclose);
    return vv;
}

G3D::Rect2D WidgetBase::GetViewRect (ViewNumber::Enum viewNumber) const
{
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = 
        CALL_MEMBER (*GetSettings (), m_getViewCount) (&mapping);
    return GetSettings ()->GetViewRect (
	m_widget->width (), m_widget->height (), mapping[viewNumber], viewCount);
}

ViewNumber::Enum WidgetBase::GetViewNumber () const
{
    return GetSettings ()->GetViewNumber ();
}

ViewSettings& WidgetBase::GetViewSettings (ViewNumber::Enum viewNumber) const
{
    return GetSettings ()->GetViewSettings (viewNumber);
}

size_t WidgetBase::GetCurrentTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetCurrentTime ();
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
	GetSettings ()->SetViewNumber (viewNumber);
}

void WidgetBase::addCopyMenu (
    QMenu* menu, const char* nameOp, 
    const boost::shared_ptr<QAction>* actionCopyOp) const
{
    if (GetSettings ()->GetViewCount () > 1)
    {
	QMenu* menuOp = menu->addMenu (nameOp);
	for (int i = 0; i < GetSettings ()->GetViewCount (); ++i)
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
    const boost::shared_ptr<QAction>* actionCopyOp) const
{
    size_t viewCount = GetSettings ()->GetViewCount ();
    bool actions = false;
    QMenu* menuOp = menu->addMenu (nameOp);
    if (viewCount > 1)
    {
        const ViewSettings& vs = GetSettings ()->GetViewSettings ();
	size_t currentProperty = vs.GetBodyOrFaceScalar ();
	for (size_t i = 0; i < viewCount; ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    const ViewSettings& otherVs = GetViewSettings (viewNumber);
	    if (viewNumber == GetViewNumber () ||
		GetSettings ()->GetColorBarType (
                    GetViewNumber ()) != 
                GetSettings ()->GetColorBarType (viewNumber) ||
		currentProperty != otherVs.GetBodyOrFaceScalar () ||
		vs.GetSimulationIndex () != otherVs.GetSimulationIndex ())
		continue;
	    menuOp->addAction (actionCopyOp[i].get ());
	    actions = true;
	}
    }
    if (! actions)
	menuOp->setDisabled (true);    
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

const Simulation& WidgetBase::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return m_simulationGroup->GetSimulation (*GetSettings (), viewNumber);
}

const Simulation& WidgetBase::GetSimulation (size_t index) const
{
    return GetSimulationGroup ().GetSimulation (index);
}

const Foam& WidgetBase::GetFoam (ViewNumber::Enum viewNumber, 
                                 size_t timeStep) const
{
    return GetSimulation (viewNumber).GetFoam (timeStep);
}
