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
#include "OpenGLUtils.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"
#include "WidgetVtk.h"

//#define __LOG__(code) code
#define __LOG__(code)

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


// Methods ViewPipeline
// ======================================================================

vtkSmartPointer<vtkRenderer> WidgetVtk::ViewPipeline::Init (
    size_t objects, size_t constraintSurfaces)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer

    // threshold
    VTK_CREATE (vtkThreshold, threshold);
    threshold->AllScalarsOn ();
    m_threshold = threshold;

    // renderer
    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);
    renderer->LightFollowCameraOn ();
    //renderer->SetAutomaticLightCreation (0);
    m_renderer = renderer;

    // scalar bar
    VTK_CREATE (vtkScalarBarActor, scalarBar);
    scalarBar->SetOrientationToVertical ();
    scalarBar->SetNumberOfLabels (0);
    m_scalarBar = scalarBar;
    renderer->AddViewProp (scalarBar);

    // scalar average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (threshold->GetOutputPort ());

    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    m_averageActor = averageActor;
    renderer->AddViewProp(averageActor);


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
    return renderer;
}

void WidgetVtk::ViewPipeline::UpdateThreshold (QwtDoubleInterval interval)
{
    if (m_threshold != 0)
    {
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
    }
}

void WidgetVtk::ViewPipeline::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    if (m_averageActor != 0)
    {
	m_scalarBar->SetLookupTable (colorTransferFunction);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
    }
}

void WidgetVtk::ViewPipeline::PositionScalarBar (G3D::Rect2D position)
{
    m_scalarBar->SetHeight (position.height ());
    m_scalarBar->SetWidth (position.width ());
    m_scalarBar->SetPosition (position.x0 (), position.y0 ());
}

void WidgetVtk::ViewPipeline::UpdateOpacity (float contextAlpha)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
    {
	actor->GetProperty ()->SetOpacity (contextAlpha);
    }
}

void WidgetVtk::ViewPipeline::UpdateFromOpenGl (
    const ViewSettings& vs, const G3D::AABox& bb, const Foam& foam)
{
    G3D::Vector3 center = bb.center ();
    G3D::Vector3 rotationCenter = vs.GetRotationCenter ();
    G3D::Vector3 position = center + G3D::Vector3 (0, 0, 1);
    G3D::Vector3 up = G3D::Vector3 (0, 1, 0);
    G3D::Matrix3 cameraRotationAxes = 
	vs.GetRotationForAxesOrder (foam).inverse ();
    G3D::Matrix3 cameraRotation = vs.GetRotation ().inverse ();
    
    // apply the rotations from ModelViewTransform in reverse order
    position = cameraRotation * (position - rotationCenter) + rotationCenter;
    up = cameraRotation * up;
    center = cameraRotation * (center - rotationCenter) + rotationCenter;

    position = cameraRotationAxes * (position - center) + center;
    up = cameraRotationAxes * up ;

    vtkCamera* camera = m_renderer->GetActiveCamera ();
    camera->SetFocalPoint (center.x, center.y, center.z);
    camera->SetPosition (position.x, position.y, position.z);
    camera->ComputeViewPlaneNormal ();
    camera->SetViewUp (up.x, up.y, up.z);

    m_renderer->ResetCamera ();
}

void WidgetVtk::ViewPipeline::UpdateAverage (
    boost::shared_ptr<RegularGridAverage> average, int direction)
{
    __LOG__ (cdbg << "UpdateAverage: " << direction[0] << endl;)

    const Foam& foam = average->GetFoam ();
    // calculate average
    average->AverageStep (direction);

    m_threshold->SetInput (average->GetAverage ());
    Foam::Bodies objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
	vtkDataSetMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInput (objects[i]->GetPolyData ());

    size_t i = 0;
    pair<size_t, Foam::OrientedFaces> p;
    BOOST_FOREACH (p, foam.GetConstraintFaces ())
    {
	vtkDataSetMapper::SafeDownCast (m_constraintSurface[i]->GetMapper ())
	    ->SetInput (foam.GetConstraintFacesPolyData (p.first));
	++i;
    }
}



// Methods WidgetVtk
// ======================================================================

WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent)
{
}

void WidgetVtk::InitAverage (boost::shared_ptr<Settings> settings,
		      const SimulationGroup& simulationGroup)    
{
    m_settings = settings;
    for (size_t i = 0; i < m_average.size (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	m_average[i].reset (new RegularGridAverage (
				viewNumber,
				*settings, simulationGroup));
    }
}

void WidgetVtk::InitPipeline (size_t objects, size_t constraintSurfaces)
{
    vtkRenderWindow* renWin = GetRenderWindow ();
    QVTKInteractor *interactor=GetInteractor();
    interactor->LightFollowCameraOn ();

    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
	m_pipeline[i].Init (objects, constraintSurfaces);

    vtkSmartPointer<SendPaintEnd> sendPaint (new SendPaintEnd (this));
    renWin->AddObserver (vtkCommand::EndEvent, sendPaint);
}

void WidgetVtk::UpdateThreshold (QwtDoubleInterval interval)
{
    m_pipeline[ m_settings->GetViewNumber ()].UpdateThreshold (interval);
    update ();
}

void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    m_pipeline[m_settings->GetViewNumber ()].UpdateColorTransferFunction (
	colorTransferFunction);
    update ();
}

void WidgetVtk::resizeEvent (QResizeEvent * event)
{
    (void) event;
    float w = width ();
    float h = height ();
    for (size_t i = 0;
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	G3D::Rect2D viewRect = m_settings->GetViewRect (w, h, viewNumber);
	G3D::Rect2D viewColorBarRect = Settings::GetViewColorBarRect (viewRect);
	G3D::Rect2D position = G3D::Rect2D::xywh (
	    (viewColorBarRect.x0 () - viewRect.x0 ())/ viewRect.width (),
	    (viewColorBarRect.y0 () - viewRect.y0 ())/ viewRect.height (),
	    viewColorBarRect.width () / viewRect.width () * 1.3,
	    viewColorBarRect.height () / viewRect.height () * 1.2);	
	m_pipeline[viewNumber].PositionScalarBar (position);
    }
}


void WidgetVtk::UpdateOpacity ()
{
    for (size_t i = 0;
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	m_pipeline[viewNumber].UpdateOpacity (m_settings->GetContextAlpha ());
    }
    update ();
}


void WidgetVtk::InitAverage ()
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    for (size_t i = 0; i < m_pipeline.size (); ++i)
	renderWindow->RemoveRenderer (m_pipeline[i].m_renderer);
}

void WidgetVtk::InitAverage (
    ViewNumber::Enum viewNumber,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
    QwtDoubleInterval interval)
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    const ViewSettings& vs = m_settings->GetViewSettings (viewNumber);
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    const Foam& foam = m_average[viewNumber]->GetFoam ();
    ViewPipeline& pipeline = m_pipeline[viewNumber];
    float w = width ();
    float h = height ();

    G3D::AABox vv = m_settings->CalculateViewingVolume (
	viewNumber, simulation, w / h);
    G3D::AABox bb = simulation.GetBoundingBox ();
    average->AverageInitStep ();
    int direction = 0;
    pipeline.UpdateAverage (average, direction);
    pipeline.UpdateFromOpenGl (vs, bb, foam);
    pipeline.UpdateOpacity (m_settings->GetContextAlpha ());
    pipeline.UpdateThreshold (interval);
    pipeline.UpdateColorTransferFunction (colorTransferFunction);
    G3D::Rect2D vr = m_settings->GetViewRect (w, h, viewNumber);
    G3D::Rect2D viewRect = G3D::Rect2D::xyxy (vr.x0 () / w, vr.y0 () / h,
					      vr.x1 () / w, vr.y1 () / h);
    pipeline.m_renderer->SetViewport (viewRect.x0 (), viewRect.y0 (),
				      viewRect.x1 (), viewRect.y1 ());
    renderWindow->AddRenderer(pipeline.m_renderer);
    resizeEvent (0);
    update ();
}

void WidgetVtk::UpdateAverage (
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    for (size_t i = 0;
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	ViewPipeline& pipeline = m_pipeline[viewNumber];
	boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
	pipeline.UpdateAverage (average, direction[i]);
    }    
}
