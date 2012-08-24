/**
 * @file   WidgetBase.h
 * @author Dan R. Lipsa
 * @date 23 August 2012
 * 
 * 
 */

#ifndef __WIDGET_BASE_H__
#define __WIDGET_BASE_H__

#include "Enums.h"
class Settings;
class Simulation;
class ViewSettings;

/**
 * Widget for displaying foam bubbles
 */
class WidgetBase
{
public:
    typedef bool (Settings::*IsViewType) (
	ViewNumber::Enum viewNumber) const;
    typedef ViewCount::Enum (Settings::*GetViewCountType) (
	vector<ViewNumber::Enum>* mapping) const;

public:
    WidgetBase (const QWidget* widget,
		IsViewType isView,
		GetViewCountType getViewCount) :
	m_widget (widget), 
	m_isView (isView),
	m_getViewCount (getViewCount)
    {
    }

    void ForAllViews (boost::function <void (ViewNumber::Enum)> f);
    void ForAllHiddenViews (boost::function <void (ViewNumber::Enum)> f);
    G3D::AABox CalculateViewingVolume (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose  = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::Rect2D GetViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetViewRect () const
    {
	return GetViewRect (GetViewNumber ());
    }

    boost::shared_ptr<Settings> GetSettings () const
    {
	return m_settings;
    }
    void SetSettings (boost::shared_ptr<Settings> settings)
    {
	m_settings = settings;
    }
    ViewNumber::Enum GetViewNumber () const;
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const;
    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }


    virtual const Simulation& GetSimulation (
	ViewNumber::Enum viewNumber) const = 0;

protected:
    void setView (const G3D::Vector2& clickedPoint);
    void setView (ViewNumber::Enum viewNumber, 
		  const G3D::Vector2& clickedPoint);

private:
    boost::shared_ptr<Settings> m_settings;
    const QWidget* m_widget;
    IsViewType m_isView;
    GetViewCountType m_getViewCount;
};


#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
