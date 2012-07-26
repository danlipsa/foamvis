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

class WidgetVtk : public QVTKWidget
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateModelView (vtkSmartPointer<vtkMatrix4x4> modelView);
    void UpdateRenderStructured (
	const Foam& foam, vtkSmartPointer<vtkMatrix4x4> modelView,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	QwtDoubleInterval interval, BodyScalar::Enum bodyScalar);
    void Init (boost::shared_ptr<Settings> settings,
	       const SimulationGroup& simulationGroup);
    void UpdateOpacity ();
    void UpdateInput (const Foam& foam, BodyScalar::Enum bodyScalar);
    void SetupPipeline (size_t objects, size_t constraintSurfaces);

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
    Q_OBJECT
    boost::shared_ptr<Settings> m_settings;
    boost::array<boost::shared_ptr<RegularGridAverage>,
		 ViewNumber::COUNT> m_average;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkActor> m_averageActor;
    vtkSmartPointer<vtkThreshold> m_threshold;
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    vector<vtkSmartPointer<vtkActor> > m_object;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
