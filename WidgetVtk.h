/**
 * @file   WidgetVtk.h
 * @author Dan R. Lipsa
 * @date 6 June 2012
 * 
 * Widget for displaying foam bubbles using VTK
 * 
 */
#ifndef __WIDGETVTK_H__
#define __WIDGETVTK_H__

#include "Enums.h"
#include "WidgetBase.h"

class Foam;
class RegularGridAverage;
class SendPaintEnd;
class Settings;
class SimulationGroup;
class AttributeAverages3D;
class ViewSettings;
class Foam;
class PipelineAverage3D;
class PipelineBase;

class WidgetVtk : public QVTKWidget, public WidgetBase
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);

    /**
     * @{
     * @name Common for all pipelines
     */
    void Init (boost::shared_ptr<Settings> settings,
               boost::shared_ptr<const SimulationGroup> simulationGroup);
    void UpdateFocus ();
    void FromViewTransform (ViewNumber::Enum viewNumber);
    void FromViewTransform ()
    {
        FromViewTransform (GetViewNumber ());
    }

    void ToViewTransform (ViewNumber::Enum viewNumber);
    void ToViewTransform ()
    {
        ToViewTransform (GetViewNumber ());
    }
    void RemoveViews ();
    PipelineType::Enum GetPipelineType (ViewNumber::Enum viewNumber);
    void ForAllViews (
        PipelineType::Enum type, boost::function <void (ViewNumber::Enum)> f);
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }
    G3D::Rect2D GetNormalizedViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetNormalizedViewRect () const
    {
        return GetNormalizedViewRect (GetViewNumber ());
    }

    void SendPaintEnd ();
    // @}

    /** 
     * @{
     * @name PipelineAverage3D
     */
    void UpdateScalarThreshold (QwtDoubleInterval interval);
    void AddAverageView (
	ViewNumber::Enum viewNumber, const ColorBarModel& scalarColorBarModel,	
	QwtDoubleInterval interval, const ColorBarModel& velocityColorBarModel);
    void FromView ();
    void UpdateAverage (ViewNumber::Enum viewNumber, int direction);
    void UpdateForceAverage ();
    void UpdateVelocityAverage ();
    void Average3dCreatePipeline (size_t objects, size_t constraintSurfaces, 
                                  size_t fontSize);
    void UpdateAverage3dTitle ();
    // @}
    
Q_SIGNALS:
    void PaintEnd ();

public Q_SLOTS:
    void CopySelectionFrom (int fromViewNumber);
    void CopyTransformationFrom (int fromViewNumber);
    void ResetTransformAll ();
    void ResetTransformFocus ();    

protected:
    virtual void resizeEvent (QResizeEvent * event);
    virtual void mousePressEvent (QMouseEvent *event);
    virtual void contextMenuEvent (QContextMenuEvent *);
    virtual void contextMenuEventView (QMenu* menu) const;

private:
    void updateViewTitle (ViewNumber::Enum viewNumber);
    void updateViewFocus (ViewNumber::Enum viewNumber);
    void resizeViewEvent (ViewNumber::Enum viewNumber);
    void removeView (ViewNumber::Enum viewNumber);
    void createActions ();

private:
    Q_OBJECT
    // current visualization pipelines
    boost::array<boost::shared_ptr<PipelineBase>, 
                 ViewNumber::COUNT> m_pipeline;
    int m_fontSize;

    // average visualization pipeline
    boost::array<boost::shared_ptr<PipelineAverage3D>, 
                 ViewNumber::COUNT> m_pipelineAverage3d;
    // average of attributes
    boost::array<boost::shared_ptr<AttributeAverages3D>,
		 ViewNumber::COUNT> m_average;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
