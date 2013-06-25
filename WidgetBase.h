/**
 * @file   WidgetBase.h
 * @author Dan R. Lipsa
 * @date 23 August 2012
 * @brief Base class for all views: WidgetGl, WidgetVtk, WidgetHistogram
 * @ingroup view
 *
 * @defgroup view View
 * Views for displaying data
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
 * @brief Base class for all views: WidgetGl, WidgetVtk, WidgetHistogram
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
     * @name Color maps for scalars and velocity
     */
    bool IsColorMapScalarCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    bool IsColorMapVelocityCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    boost::shared_ptr<QAction> GetActionColorMapScalarEdit () const
    {
	return m_actionColorMapScalarEdit;
    }
    boost::shared_ptr<QAction> GetActionColorMapScalarClampClear () const
    {
	return m_actionColorMapScalarClampClear;
    }
    boost::shared_ptr<QAction> GetActionColorMapVelocityEdit () const
    {
	return m_actionColorMapVelocityEdit;
    }
    boost::shared_ptr<QAction> GetActionColorMapVelocityClampClear () const
    {
	return m_actionColorMapVelocityClampClear;
    }
    boost::shared_ptr<QAction> 
    GetActionColorMapVelocityCopyVelocityMagnitude () const
    {
	return m_actionColorMapVelocityCopyVelocityMagnitude;
    }

    boost::shared_ptr<QSignalMapper> GetSignalMapperColorMapScalarCopy () const
    {
	return m_signalMapperColorMapScalarCopy;
    }
    boost::shared_ptr<QSignalMapper> GetSignalMapperColorMapVelocityCopy () const
    {
	return m_signalMapperColorMapVelocityCopy;
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
    QMenu* addCopyCompatibleMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp,
        IsCopyCompatibleType isCopyCompatible) const;
    void initCopy (
	boost::array<boost::shared_ptr<QAction>, 
	ViewNumber::COUNT>& actionCopyTransform,
	boost::shared_ptr<QSignalMapper>& signalMapperCopyTransform);
    void infoFoam ();
    void infoSimulation ();

    virtual void contextMenuEventView (QMenu* menu) const
    {(void)menu;}
    virtual void contextMenuEventColorMapScalar (QMenu* menu) const;
    virtual void contextMenuEventColorMapVelocity (QMenu* menu) const;

protected:
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyTransform;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyTransform;

    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyForceRatio;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyForceRatio;

    
    boost::array<boost::shared_ptr<QAction>, 
                 ViewNumber::COUNT> m_actionCopySelection;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopySelection;

    boost::shared_ptr<QAction> m_actionResetTransformAll;
    boost::shared_ptr<QAction> m_actionResetTransformFocus;

    // color bars actions
    boost::shared_ptr<QAction> m_actionColorMapScalarEdit;
    boost::shared_ptr<QAction> m_actionColorMapScalarClampClear;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionColorMapScalarCopy;
    boost::shared_ptr<QSignalMapper> m_signalMapperColorMapScalarCopy;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionColorMapVelocityCopy;
    boost::shared_ptr<QSignalMapper> m_signalMapperColorMapVelocityCopy;
    // color map velocity actions
    boost::shared_ptr<QAction> m_actionColorMapVelocityEdit;
    boost::shared_ptr<QAction> m_actionColorMapVelocityClampClear;
    boost::shared_ptr<QAction> m_actionColorMapVelocityCopyVelocityMagnitude;

    boost::shared_ptr<QAction> m_actionInfoFoam;
    boost::shared_ptr<QAction> m_actionInfoSimulation;

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
    connect (m_signalMapperCopyTransform.get (),\
	     SIGNAL (mapped (int)),\
	     this,\
	     SLOT (CopyTransformFromSlot (int)));\
    connect (m_signalMapperCopyForceRatio.get (), \
	     SIGNAL (mapped (int)),\
	     this,\
	     SLOT (CopyForceRatioFromSlot (int)));                      \
    connect (m_actionResetTransformAll.get (), SIGNAL(triggered()),\
             this, SLOT(ResetTransformAll ()));\
    connect (m_actionResetTransformFocus.get (), SIGNAL(triggered()),\
	    this, SLOT(ResetTransformFocus ()));\
    connect(m_actionInfoFoam.get (), SIGNAL(triggered()), this,\
	    SLOT(InfoFoam ()));\
    connect(m_actionInfoSimulation.get (), SIGNAL(triggered()), this,   \
	    SLOT(InfoSimulation ()));



#endif //__WIDGET_BASE_H__

// Local Variables:
// mode: c++
// End:
