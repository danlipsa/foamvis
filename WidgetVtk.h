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
#include "WidgetDisplay.h"

class Foam;
class SendPaintEnd;
class Settings;
class SimulationGroup;
class RegularGridAverage;
class ViewSettings;
class Foam;

class WidgetVtk : public QVTKWidget, public WidgetDisplay
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	const char* name);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateFromOpenGl (vtkSmartPointer<vtkMatrix4x4> modelView);
    void RemoveViews ();
    void AddView (
	ViewNumber::Enum viewNumber,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	QwtDoubleInterval interval);
    void CreateAverage (boost::shared_ptr<Settings> settings,
		      const SimulationGroup& simulationGroup);
    void UpdateOpacity ();
    void UpdateAverage (const boost::array<int, ViewNumber::COUNT>& direction);
    void CreateViewPipelines (size_t objects, size_t constraintSurfaces, 
			      size_t fontSize);
    void UpdateTitle ();
    void UpdateFocus ();
    
    virtual const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;

public:
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }
    const RegularGridAverage& GetScalarAverage (
	ViewNumber::Enum viewNumber) const
    {
	return *m_average[viewNumber];
    }

protected:
    virtual void resizeEvent (QResizeEvent * event);

Q_SIGNALS:
    void PaintEnd ();

public Q_SLOTS:
    void updateCurrentView (vtkObject* obj);

private:
    struct ViewPipeline
    {
    public:
	vtkSmartPointer<vtkRenderer> Init (
	    size_t objects, size_t constraintSurfaces, size_t fontSize);
	void UpdateThreshold (QwtDoubleInterval interval);
	void UpdateColorTransferFunction (
	    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	    const char* name);
	void PositionScalarBar (G3D::Rect2D position);
	void UpdateOpacity (float contextAlpha);
	void UpdateFromOpenGl (
	    const ViewSettings& vs, const G3D::AABox& bb, const Foam& foam);
	void UpdateAverage (
	    boost::shared_ptr<RegularGridAverage> average, int direction);
	void UpdateTitle (
	    bool titleShown, boost::shared_ptr<RegularGridAverage> average, 
	    ViewNumber::Enum viewNumber);
	void UpdateFocus (bool focus);

    public:
	vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
	vtkSmartPointer<vtkActor> m_averageActor;
	vtkSmartPointer<vtkThreshold> m_threshold;
	vtkSmartPointer<vtkTextMapper> m_textMapper;
	vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
	vector<vtkSmartPointer<vtkActor> > m_object;
	vtkSmartPointer<vtkRenderer> m_renderer;
	vtkSmartPointer<vtkActor2D> m_focusActor;
    };

private:
    void updateViewAverage (
	ViewNumber::Enum viewNumber,
	const boost::array<int, ViewNumber::COUNT>& direction);
    void updateViewTitle (ViewNumber::Enum viewNumber);
    void updateViewFocus (ViewNumber::Enum viewNumber);

private:
    Q_OBJECT
    boost::array<boost::shared_ptr<RegularGridAverage>,
		 ViewNumber::COUNT> m_average;
    boost::array<ViewPipeline, ViewNumber::COUNT> m_pipeline;
    vtkSmartPointer<vtkEventQtSlotConnect> m_connections;
    int m_fontSize;
    
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
