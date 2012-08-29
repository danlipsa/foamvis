/**
 * @file   WidgetBase.cpp
 * @author Dan R. Lipsa
 * @date 23 August 2012
 * 
 * Definitions for the widget for displaying foam bubbles
 */


#include "Debug.h"
#include "Settings.h"
#include "Utils.h"
#include "WidgetBase.h"


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
    ViewCount::Enum viewCount = CALL_MEMBER (
	*GetSettings (), m_getViewCount) (&mapping);
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

