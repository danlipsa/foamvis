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
class ColorBarModel;
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
    typedef boost::array<boost::shared_ptr<AverageCache>, 
                         ViewNumber::COUNT> AverageCaches;

public:
    WidgetBase (QWidget* widget,
		IsViewType isView,
		GetViewCountType getViewCount);
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

    const Foam& GetFoam (ViewNumber::Enum viewNumber, size_t timeStep) const;
    const Foam& GetFoam (ViewNumber::Enum viewNumber) const
    {
        return GetFoam (viewNumber, GetTime (viewNumber));
    }
    const Foam& GetFoam () const
    {
        return GetFoam (GetViewNumber ());
    }

    bool IsColorBarCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    bool IsSelectionCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;

protected:
    typedef bool (WidgetBase::*IsCopyCompatibleType) (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    
protected:
    void setView (const G3D::Vector2& clickedPoint);
    void setView (ViewNumber::Enum viewNumber, 
		  const G3D::Vector2& clickedPoint);
    void addCopyMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp) const;
    void addCopyCompatibleMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp,
        IsCopyCompatibleType isCopyCompatible) const;
    void initCopy (
	boost::array<boost::shared_ptr<QAction>, 
	ViewNumber::COUNT>& actionCopyTransformation,
	boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformation);


protected:
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyTransformation;
boost::shared_ptr<QSignalMapper> m_signalMapperCopyTransformation;

    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopySelection;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopySelection;

private:
    AverageCaches* m_averageCache;
    QWidget* m_widget;
    IsViewType m_isView;
    GetViewCountType m_getViewCount;
};


#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
