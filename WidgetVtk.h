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
class SendPaintEnd;
class Settings;
class SimulationGroup;
class RegularGridAverage;
class ViewSettings;
class Foam;
class Average3dPipeline;

class WidgetVtk : public QVTKWidget, public WidgetBase
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	const char* name);
    void UpdateThreshold (QwtDoubleInterval interval);
    void ViewToVtk (ViewNumber::Enum viewNumber);
    void VtkToView (ViewNumber::Enum viewNumber);
    void VtkToView ()
    {
        VtkToView (GetViewNumber ());
    }
    void RemoveViews ();
    void AddView (
	ViewNumber::Enum viewNumber,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	QwtDoubleInterval interval);
    void CreateAverage (boost::shared_ptr<Settings> settings,
		      const SimulationGroup& simulationGroup);
    void UpdateOpacity ();
    void UpdateAverage (const boost::array<int, ViewNumber::COUNT>& direction);
    void UpdateViewAverage (
        ViewNumber::Enum viewNumber,
        const boost::array<int, ViewNumber::COUNT>& direction);
    void CreateAverage3dPipeline (size_t objects, size_t constraintSurfaces, 
                                  size_t fontSize);
    void UpdateTitle ();
    void UpdateFocus ();
    
    virtual const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }
    RegularGridAverage& GetScalarAverage (ViewNumber::Enum viewNumber)
    {
	return *m_average[viewNumber];
    }

    G3D::Rect2D GetNormalizedViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetNormalizedViewRect () const
    {
        return GetNormalizedViewRect (GetViewNumber ());
    }

    void SendPaintEnd ();

Q_SIGNALS:
    void PaintEnd ();

public Q_SLOTS:
    void CopyTransformFrom (int fromViewNumber);

protected:
    virtual void resizeEvent (QResizeEvent * event);
    virtual void mousePressEvent (QMouseEvent *event);
    virtual void contextMenuEvent (QContextMenuEvent *);

private:
    void updateViewTitle (ViewNumber::Enum viewNumber);
    void updateViewFocus (ViewNumber::Enum viewNumber);
    void resizeViewEvent (ViewNumber::Enum viewNumber);
    void updateViewOpacity (ViewNumber::Enum viewNumber);

private:
    Q_OBJECT
    boost::array<boost::shared_ptr<RegularGridAverage>,
		 ViewNumber::COUNT> m_average;
    boost::array<boost::shared_ptr<Average3dPipeline>, 
                 ViewNumber::COUNT> m_pipeline;
    int m_fontSize;
    
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
