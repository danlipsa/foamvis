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
class AverageCache;
class Settings;
class Simulation;
class SimulationGroup;
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
    WidgetBase (QWidget* widget,
		IsViewType isView,
		GetViewCountType getViewCount) :
        m_simulationGroup (0),
        m_averageCache (0),
	m_widget (widget), 
	m_isView (isView),
	m_getViewCount (getViewCount)
    {
    }

    void Init (boost::shared_ptr<Settings> settings,
	       const SimulationGroup* simulationGroup, 
               AverageCache* averageCache);

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
    const SimulationGroup& GetSimulationGroup () const
    {
	return *m_simulationGroup;
    }
    AverageCache* GetAverageCache ()
    {
        return m_averageCache;
    }
    
    ViewNumber::Enum GetViewNumber () const;
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const;
    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }
    size_t GetCurrentTime () const
    {
	return GetCurrentTime (GetViewNumber ());
    }
    size_t GetCurrentTime (ViewNumber::Enum viewNumber) const;



    const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation (size_t index) const;
    const Simulation& GetSimulation () const
    {
        return GetSimulation (GetViewNumber ());
    }



protected:
    void setView (const G3D::Vector2& clickedPoint);
    void setView (ViewNumber::Enum viewNumber, 
		  const G3D::Vector2& clickedPoint);
    void addCopyMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp) const;
    void addCopyCompatibleMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp) const;
    void initCopy (
	boost::array<boost::shared_ptr<QAction>, 
	ViewNumber::COUNT>& actionCopyTransformation,
	boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformation);

protected:
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyTransformation;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyTransformation;

private:
    boost::shared_ptr<Settings> m_settings;
    const SimulationGroup* m_simulationGroup;
    AverageCache* m_averageCache;    
    QWidget* m_widget;
    IsViewType m_isView;
    GetViewCountType m_getViewCount;
};


#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
