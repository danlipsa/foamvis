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
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
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

void WidgetVtk::Init (boost::shared_ptr<Settings> settings,
		      const SimulationGroup& simulationGroup)    
{
    m_settings = settings;
    for (size_t i = 0; i < m_average.size (); ++i)
	m_average[i].reset (new RegularGridAverage (*settings, simulationGroup));
}

void WidgetVtk::SetupPipeline (size_t objects, size_t constraintSurfaces)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    vtkSmartPointer<SendPaintEnd> sendPaint (new SendPaintEnd (this));

    // threshold
    VTK_CREATE (vtkThreshold, threshold);
    threshold->AllScalarsOn ();
    m_threshold = threshold;

    // renderer
    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);

    // scalar bar
    VTK_CREATE (vtkScalarBarActor, scalarBar);
    scalarBar->SetOrientationToVertical ();
    scalarBar->SetNumberOfLabels (0);
    m_scalarBar = scalarBar;
    renderer->AddViewProp (scalarBar);

    // average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (threshold->GetOutputPort ());

    // scalar average
    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    renderer->AddViewProp(averageActor);
    m_averageActor = averageActor;

    // foam objects
    m_object.resize (objects);
    for (size_t i = 0; i < objects; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_object[i] = actor;
	renderer->AddViewProp (actor);
    }

    // constraint faces rendered transparent
    m_constraintSurface.resize (constraintSurfaces);
    for (size_t i = 0; i < constraintSurfaces; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_constraintSurface[i] = actor;
	renderer->AddViewProp (actor);
    }

    GetRenderWindow()->AddRenderer(renderer);
    GetRenderWindow ()->AddObserver (vtkCommand::EndEvent, sendPaint);
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
    if (m_averageActor != 0)
    {
	m_scalarBar->SetLookupTable (colorTransferFunction);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
	update ();
    }
}

void WidgetVtk::resizeEvent (QResizeEvent * event)
{
    (void) event;
    G3D::Rect2D viewRect = m_settings->GetViewRect (width (), height ());
    G3D::Rect2D viewColorBarRect = Settings::GetViewColorBarRect (viewRect);
    float heightRatio = viewColorBarRect.height () / viewRect.height ();
    float widthRatio = viewColorBarRect.width () / viewRect.width ();
    /*
    cdbg << viewRect << endl;
    cdbg << viewColorBarRect << endl;
    cdbg << heightRatio << ", " << widthRatio << endl;    
    */
    m_scalarBar->SetHeight (heightRatio * 1.2);
    m_scalarBar->SetWidth (widthRatio * 1.3);
    m_scalarBar->SetPosition (viewColorBarRect.x0 () / viewRect.width (),
			      viewColorBarRect.y0 () / viewRect.height ());
}


void WidgetVtk::UpdateOpacity ()
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
    {
	actor->GetProperty ()->SetOpacity (m_settings->GetContextAlpha ());
    }
    update ();
}

void WidgetVtk::UpdateModelView (vtkSmartPointer<vtkMatrix4x4> modelView)
{
    m_averageActor->SetUserMatrix (modelView);
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_object)
	actor->SetUserMatrix (modelView);
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
	actor->SetUserMatrix (modelView);
}

void WidgetVtk::UpdateInput (const Foam& foam, BodyScalar::Enum bodyScalar)
{
    vtkSmartPointer<vtkImageData> regularFoam = foam.GetRegularGrid ();
    regularFoam->GetPointData ()->SetActiveScalars (
	BodyScalar::ToString (bodyScalar));
    m_threshold->SetInput (regularFoam);

    Foam::Bodies objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
	vtkDataSetMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInput (objects[i]->GetPolyData ());

    for (size_t i = 0; i < foam.GetConstraintFacesSize (); ++i)
	vtkDataSetMapper::SafeDownCast (m_constraintSurface[i]->GetMapper ())
	    ->SetInput (foam.GetConstraintFacesPolyData (i));
}

void WidgetVtk::UpdateRenderStructured (
    const Foam& foam, vtkSmartPointer<vtkMatrix4x4> modelView,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
    QwtDoubleInterval interval, BodyScalar::Enum bodyScalar)
{
    UpdateInput (foam, bodyScalar);
    UpdateModelView (modelView);
    UpdateOpacity ();
    UpdateThreshold (interval);
    UpdateColorTransferFunction (colorTransferFunction);
    update ();
}

