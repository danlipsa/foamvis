/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "Body.h"
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

    // renderer
    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);

    // scalar bar
    VTK_CREATE (vtkScalarBarActor, scalarBar);
    scalarBar->SetLookupTable (colorTransferFunction);
    scalarBar->SetOrientationToVertical ();
    scalarBar->SetHeight (0.8);
    scalarBar->SetWidth (0.17);
    renderer->AddViewProp (scalarBar);

    // average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (threshold->GetOutputPort ());
    averageMapper->SetLookupTable (colorTransferFunction);
    m_mapper = averageMapper;

    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    averageActor->SetUserMatrix (modelView);
    renderer->AddViewProp(averageActor);


    // foam objects mappers and actors
    Foam::Bodies objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);
	mapper->SetInput (objects[i]->GetPolyData ());

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	actor->SetUserMatrix (modelView);
	renderer->AddViewProp (actor);
    }

    // constraint faces rendered transparent
    for (size_t i = 0; i < foam.GetConstraintFacesSize (); ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);
	mapper->SetInput (foam.GetConstraintFacesPolyData (i));

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	actor->SetUserMatrix (modelView);
	actor->GetProperty ()->SetOpacity (0.05);
	renderer->AddViewProp (actor);
    }

    GetRenderWindow()->AddRenderer(renderer);
    GetRenderWindow ()->AddObserver (vtkCommand::EndEvent, sendPaint);
    update ();
}

