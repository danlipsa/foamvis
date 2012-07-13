/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "BodySelector.h"
#include "DebugStream.h"
#include "Foam.h"
#include "WidgetVtk.h"

// Private Classes/Functions
// ======================================================================

class SendPaintEnd : public vtkCommand
{
public:
    SendPaintEnd (WidgetVtk* widgetVtk) :
	m_widgetVtk (widgetVtk)
    {
    }
    void Execute (vtkObject *caller, unsigned long eventId, void *callData);
private:
    WidgetVtk* m_widgetVtk;
};


void SendPaintEnd::Execute (
    vtkObject *caller, unsigned long eventId, void *callData)
{
    (void) caller;(void)callData;
    if (eventId == vtkCommand::EndEvent)
	Q_EMIT m_widgetVtk->PaintEnd ();
}


// Methods
// ======================================================================

WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent)
{
}

void WidgetVtk::UpdateThreshold (QwtDoubleInterval interval)
{
    if (m_threshold != 0)
    {
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
	update ();
    }
}

void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    if (m_mapper != 0)
    {
	m_mapper->SetLookupTable (colorTransferFunction);
	update ();
    }
}

void WidgetVtk::UpdateRenderStructured (
    const Foam& foam, vtkSmartPointer<vtkMatrix4x4> modelView,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
    QwtDoubleInterval interval, BodyScalar::Enum bodyScalar)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    vtkSmartPointer<SendPaintEnd> sendPaint (new SendPaintEnd (this));

    vtkSmartPointer<vtkImageData> regularFoam = foam.GetRegularGrid ();
    regularFoam->GetPointData ()->SetActiveScalars (
	BodyScalar::ToString (bodyScalar));

    VTK_CREATE (vtkThreshold, threshold);
    threshold->ThresholdBetween (interval.minValue (), interval.maxValue ());
    threshold->AllScalarsOn ();
    //threshold->SetInputConnection (regularProbe->GetOutputPort ());
    threshold->SetInput (regularFoam);
    m_threshold = threshold;

    VTK_CREATE (vtkDataSetMapper, mapper);
    mapper->SetInputConnection (threshold->GetOutputPort ());
    mapper->SetLookupTable (colorTransferFunction);
    m_mapper = mapper;

    VTK_CREATE(vtkActor, actor);
    actor->SetMapper(mapper);
    actor->SetUserMatrix (modelView);

    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);
    renderer->AddViewProp(actor);

    GetRenderWindow()->AddRenderer(renderer);
    GetRenderWindow ()->AddObserver (vtkCommand::EndEvent, sendPaint);
    update ();
}

