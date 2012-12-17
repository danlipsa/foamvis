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
#include "Base.h"
class AverageCache;
class Foam;
class Settings;
class Simulation;
class SimulationGroup;
class ViewSettings;

/**
 * Widget for displaying foam bubbles
 */
class WidgetBase : public Base
{
public:
    typedef bool (Settings::*IsViewType) (
	ViewNumber::Enum viewNumber) const;
    typedef ViewCount::Enum (Settings::*GetViewCountType) (
	vector<ViewNumber::Enum>* mapping) const;
    typedef boost::array<boost::shared_ptr<AverageCache>, 
                         ViewNumber::COUNT> AverageCaches;

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
               AverageCaches* averageCache);

    void ForAllViews (boost::function <void (ViewNumber::Enum)> f);
    void ForAllHiddenViews (boost::function <void (ViewNumber::Enum)> f);
    G3D::AABox CalculateViewingVolume (
	ViewNumber::Enum viewNumber, const Simulation& simulation,
	ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::Rect2D GetViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetViewRect () const
    {
	return GetViewRect (GetViewNumber ());
    }

    const SimulationGroup& GetSimulationGroup () const
    {
	return *m_simulationGroup;
    }
    boost::shared_ptr<AverageCache> GetAverageCache (
        ViewNumber::Enum viewNumber) const
    {
        return (*m_averageCache)[viewNumber];
    }
    
    G3D::Matrix3 GetRotationForAxesOrder (ViewNumber::Enum viewNumber, 
                                          size_t timeStep) const;
    G3D::Matrix3 GetRotationForAxesOrder (ViewNumber::Enum viewNumber) const
    {
        return GetRotationForAxesOrder (viewNumber, GetTime (viewNumber));
    }

    const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation (size_t index) const;
    const Simulation& GetSimulation () const
    {
        return GetSimulation (GetViewNumber ());
    }
    const Foam& GetFoam (ViewNumber::Enum viewNumber, size_t timeStep) const;
    const Foam& GetFoam (ViewNumber::Enum viewNumber) const
    {
        return GetFoam (viewNumber, GetTime (viewNumber));
    }
    const Foam& GetFoam () const
    {
        return GetFoam (GetViewNumber ());
    }

protected:
    void setView (const G3D::Vector2& clickedPoint);
    void setView (ViewNumber::Enum viewNumber, 
		  const G3D::Vector2& clickedPoint);
    void addCopyMenu (
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
    const SimulationGroup* m_simulationGroup;
    AverageCaches* m_averageCache;
    QWidget* m_widget;
    IsViewType m_isView;
    GetViewCountType m_getViewCount;
};


#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
