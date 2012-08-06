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

class Foam;
class SendPaintEnd;
class Settings;
class SimulationGroup;
class RegularGridAverage;
class ViewSettings;

class WidgetVtk : public QVTKWidget
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateFromOpenGl (vtkSmartPointer<vtkMatrix4x4> modelView);
    void InitAverage ();
    void InitAverage (
	ViewNumber::Enum viewNumber,
	vtkSmartPointer<vtkMatrix4x4> modelView,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	QwtDoubleInterval interval);
    void Init (boost::shared_ptr<Settings> settings,
	       const SimulationGroup& simulationGroup);
    void UpdateOpacity ();
    void UpdateAverage (const boost::array<int, ViewNumber::COUNT>& direction);
    void InitPipeline (size_t objects, size_t constraintSurfaces);

public:
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }

protected:
    virtual void resizeEvent (QResizeEvent * event);

Q_SIGNALS:
    void PaintEnd ();

private:
    struct ViewPipeline
    {
    public:
	vtkSmartPointer<vtkRenderer> Init (
	    size_t objects, size_t constraintSurfaces);
	void UpdateThreshold (QwtDoubleInterval interval);
	void UpdateColorTransferFunction (
	    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
	void PositionScalarBar (G3D::Rect2D position);
	void UpdateOpacity (float contextAlpha);
	void UpdateFromOpenGl (
	    vtkSmartPointer<vtkMatrix4x4> modelView, const ViewSettings& vs);
	void UpdateAverage (
	    boost::shared_ptr<RegularGridAverage> average, int direction);

    public:
	vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
	vtkSmartPointer<vtkActor> m_averageActor;
	vtkSmartPointer<vtkThreshold> m_threshold;
	vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
	vector<vtkSmartPointer<vtkActor> > m_object;
	vtkSmartPointer<vtkRenderer> m_renderer;
    };

private:
    Q_OBJECT
    boost::shared_ptr<Settings> m_settings;
    boost::array<boost::shared_ptr<RegularGridAverage>,
		 ViewNumber::COUNT> m_average;
    boost::array<ViewPipeline, ViewNumber::COUNT> m_pipeline;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
