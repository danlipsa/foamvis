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
    typedef boost::shared_ptr<QAction> (WidgetBase::*GetActionType) () const;
        
public:
    WidgetBase (QWidget* widget,
		IsViewType isView,
		GetViewCountType getViewCount);

    QString tr (const char * sourceText, 
                const char * disambiguation = 0, int n = -1);
    void Init (boost::shared_ptr<Settings> settings,
	       boost::shared_ptr<const SimulationGroup> simulationGroup, 
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
    
    G3D::Matrix3 GetRotationForAxisOrder (ViewNumber::Enum viewNumber, 
                                          size_t timeStep) const;
    G3D::Matrix3 GetRotationForAxisOrder (ViewNumber::Enum viewNumber) const
    {
        return GetRotationForAxisOrder (viewNumber, GetTime (viewNumber));
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

    bool IsSelectionCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    void ResetTransformFocus ();
    void contextMenuEvent (QContextMenuEvent *event);
    float GetDeformationSizeInitialRatio (ViewNumber::Enum viewNumber) const;
    // bubbleDiameter / velocityRange
    float GetVelocitySizeInitialRatio (ViewNumber::Enum viewNumber) const;

    /**
     * @{
     * @name Color and Overlay Maps
     */
    bool IsColorMapCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    bool IsOverlayMapCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    boost::shared_ptr<QAction> GetActionColorMapEdit () const
    {
	return m_actionColorMapEdit;
    }
    boost::shared_ptr<QAction> GetActionColorMapClampClear () const
    {
	return m_actionColorMapClampClear;
    }
    boost::shared_ptr<QAction> GetActionOverlayMapEdit () const
    {
	return m_actionOverlayMapEdit;
    }
    boost::shared_ptr<QAction> GetActionOverlayMapClampClear () const
    {
	return m_actionOverlayMapClampClear;
    }
    boost::shared_ptr<QSignalMapper> GetSignalMapperColorMapCopy () const
    {
	return m_signalMapperColorMapCopy;
    }
    boost::shared_ptr<QSignalMapper> GetSignalMapperOverlayMapCopy () const
    {
	return m_signalMapperOverlayMapCopy;
    }
    boost::shared_ptr<QAction> GetActionOverlayMapCopyVelocityMagnitude () const
    {
	return m_actionOverlayMapCopyVelocityMagnitude;
    }
    // @}

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
    virtual void contextMenuEventView (QMenu* menu) const
    {(void)menu;}
    virtual void contextMenuEventColorMap (QMenu* menu) const;
    virtual void contextMenuEventOverlayMap (QMenu* menu) const;

protected:
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyTransformation;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyTransformation;
    
    boost::array<boost::shared_ptr<QAction>, 
                 ViewNumber::COUNT> m_actionCopySelection;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopySelection;

    boost::shared_ptr<QAction> m_actionResetTransformAll;
    boost::shared_ptr<QAction> m_actionResetTransformFocus;

    // color bars actions
    boost::shared_ptr<QAction> m_actionColorMapEdit;
    boost::shared_ptr<QAction> m_actionColorMapClampClear;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionColorMapCopy;
    boost::shared_ptr<QSignalMapper> m_signalMapperColorMapCopy;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionOverlayMapCopy;
    boost::shared_ptr<QSignalMapper> m_signalMapperOverlayMapCopy;
    // overlay bar actions
    boost::shared_ptr<QAction> m_actionOverlayMapEdit;
    boost::shared_ptr<QAction> m_actionOverlayMapClampClear;
    boost::shared_ptr<QAction> m_actionOverlayMapCopyVelocityMagnitude;

private:
    AverageCaches* m_averageCache;
    QWidget* m_widget;
    IsViewType m_isView;
    GetViewCountType m_getViewCount;
};

#define MAKE_COMMON_CONNECTIONS\
    connect (m_signalMapperCopySelection.get (),\
             SIGNAL (mapped (int)),\
             this,\
             SLOT (CopySelectionFrom (int)));\
    connect (m_signalMapperCopyTransformation.get (),\
	     SIGNAL (mapped (int)),\
	     this,\
	     SLOT (CopyTransformationFrom (int)));\
    connect (m_actionResetTransformAll.get (), SIGNAL(triggered()),\
             this, SLOT(ResetTransformAll ()));\
    connect (m_actionResetTransformFocus.get (), SIGNAL(triggered()),\
	    this, SLOT(ResetTransformFocus ()));

#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
