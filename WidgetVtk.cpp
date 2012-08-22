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
#include "vtkConeSource.h"

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
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    //                                      vtkScalarBarActor 
    // 
    // (foam objects)    vtkPolyData->vtkDatasetMapper->vtkActor
    // (constraint faces)


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
    scalarBar->SetNumberOfLabels (3);
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

    // view title
    VTK_CREATE (vtkTextProperty, singleLineTextProp);
    singleLineTextProp->SetFontSize (fontSize);
    singleLineTextProp->SetFontFamilyToArial ();
    singleLineTextProp->BoldOff ();
    singleLineTextProp->ItalicOff ();
    singleLineTextProp->ShadowOff ();
    
    VTK_CREATE(vtkTextProperty, multiLineTextProp);
    multiLineTextProp->ShallowCopy (singleLineTextProp);
    multiLineTextProp->ShadowOn ();
    multiLineTextProp->SetLineSpacing (1);

    VTK_CREATE(vtkTextMapper, textMapper);
    vtkTextProperty* tprop = textMapper->GetTextProperty ();
    tprop->ShallowCopy (multiLineTextProp);
    tprop->SetJustificationToCentered ();
    tprop->SetVerticalJustificationToTop ();
    tprop->SetColor (0, 0, 0);
    m_textMapper = textMapper;
    
    VTK_CREATE (vtkActor2D, textActor);
    textActor->SetMapper (textMapper);
    textActor->GetPositionCoordinate ()->
	SetCoordinateSystemToNormalizedDisplay ();
    textActor->GetPositionCoordinate ()->SetValue (0.5, 1);
    renderer->AddViewProp (textActor);

    // focus rectangle
    VTK_CREATE(vtkPoints, Pts);
    Pts->InsertNextPoint (0.0, 0.0, 0.0);
    Pts->InsertNextPoint (0.0, 1.0, 0.0);
    Pts->InsertNextPoint (1.0, 1.0, 0.0);
    Pts->InsertNextPoint (1.0, 0.0, 0.0);

    VTK_CREATE (vtkCellArray, Lines);
    Lines->InsertNextCell (2);
    Lines->InsertCellPoint (0);
    Lines->InsertCellPoint (1);
    Lines->InsertNextCell (2);
    Lines->InsertCellPoint (1);
    Lines->InsertCellPoint (2);
    Lines->InsertNextCell (2);
    Lines->InsertCellPoint (2);
    Lines->InsertCellPoint (3);
    Lines->InsertNextCell (2);
    Lines->InsertCellPoint (3);
    Lines->InsertCellPoint (0);

    VTK_CREATE (vtkPolyData, Grid);
    Grid->SetPoints (Pts);
    Grid->SetLines (Lines);

    VTK_CREATE (vtkCoordinate, normCoords);
    normCoords->SetCoordinateSystemToNormalizedViewport ();

    VTK_CREATE (vtkPolyDataMapper2D, mapper);
    mapper->SetInput (Grid);
    mapper->SetTransformCoordinate (normCoords);
    
    VTK_CREATE (vtkActor2D, focusActor);
    focusActor->SetMapper (mapper);
    focusActor->GetProperty ()->SetColor (0.1, 0.1, 0.1);
    m_focusActor = focusActor;

    return renderer;
}

void WidgetVtk::ViewPipeline::UpdateTitle (bool titleShown,
    boost::shared_ptr<RegularGridAverage> average, ViewNumber::Enum viewNumber)
{
    const char* title = "";
    ostringstream ostr;
    if (titleShown)
    {	
	ostr << average->GetSimulation ().GetName () << endl 
	     << average->GetViewSettings ().GetTitle (viewNumber);
	title = ostr.str ().c_str ();
    }
    m_textMapper->SetInput (title);
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
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char * name)
{
    if (m_averageActor != 0)
    {
	m_scalarBar->SetLookupTable (colorTransferFunction);
	m_scalarBar->SetTitle (name);
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

void WidgetVtk::ViewPipeline::UpdateFocus (bool focus)
{
    if (focus)
	m_renderer->AddViewProp (m_focusActor);
    else
	m_renderer->RemoveViewProp (m_focusActor);
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
    const ViewSettings& vs = average->GetViewSettings ();
    // calculate average
    average->AverageStep (direction);

    m_threshold->SetInput (average->GetAverage ());

    Foam::Bodies objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
	vtkDataSetMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInput (objects[i]->GetPolyData ());
	if (vs.IsAverageAround ())
	{
	    G3D::Vector3 t = average->GetTranslation ();
	    m_object[i]->SetPosition (t.x, t.y, t.z);
	}
    }

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
    QVTKWidget (parent),
    m_fontSize (10)
{
    setVisible (false);

    VTK_CREATE (vtkEventQtSlotConnect, Connections);
    m_connections = Connections;

    Connections->Connect(GetRenderWindow()->GetInteractor(),
			 vtkCommand::MouseMoveEvent,
			 this,
			 SLOT(updateCoords(vtkObject*)));
}

void WidgetVtk::updateCoords (vtkObject* obj)
{
    // get interactor
    vtkRenderWindowInteractor* iren = 
	vtkRenderWindowInteractor::SafeDownCast(obj);
    // get event position
    int event_pos[2];
    iren->GetEventPosition(event_pos);

}

void WidgetVtk::CreateAverage (boost::shared_ptr<Settings> settings,
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

void WidgetVtk::CreateViewPipelines (
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    vtkRenderWindow* renWin = GetRenderWindow ();

    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
	m_pipeline[i].Init (objects, constraintSurfaces, fontSize);

    vtkSmartPointer<SendPaintEnd> sendPaint (new SendPaintEnd (this));
    renWin->AddObserver (vtkCommand::EndEvent, sendPaint);
}

void WidgetVtk::UpdateThreshold (QwtDoubleInterval interval)
{
    m_pipeline[ m_settings->GetViewNumber ()].UpdateThreshold (interval);
    update ();
}

void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char* name)
{
    m_pipeline[m_settings->GetViewNumber ()].UpdateColorTransferFunction (
	colorTransferFunction, name);
    update ();
}

void WidgetVtk::resizeEvent (QResizeEvent * event)
{
    QVTKWidget::resizeEvent (event);
    (void) event;
    for (int i = 0; i < m_settings->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (m_settings->IsVtkView (viewNumber))
	{
	    G3D::Rect2D viewRect = GetViewRect (viewNumber);
	    G3D::Rect2D viewColorBarRect = 
		Settings::GetViewColorBarRect (viewRect);
	    G3D::Rect2D position = G3D::Rect2D::xywh (
		(viewColorBarRect.x0 () - viewRect.x0 ())/ viewRect.width (),
		(viewColorBarRect.y0 () - viewRect.y0 ())/ viewRect.height (),
		viewColorBarRect.width () / viewRect.width () * 5,
		viewColorBarRect.height () / viewRect.height () * 1.2);	
	    m_pipeline[viewNumber].PositionScalarBar (position);
	}
    }
}


void WidgetVtk::UpdateOpacity ()
{
    for (int i = 0; i < m_settings->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	m_pipeline[viewNumber].UpdateOpacity (m_settings->GetContextAlpha ());
    }
    update ();
}


void WidgetVtk::RemoveViews ()
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    for (size_t i = 0; i < m_pipeline.size (); ++i)
	renderWindow->RemoveRenderer (m_pipeline[i].m_renderer);
    setVisible (false);
}

void WidgetVtk::AddView (
    ViewNumber::Enum viewNumber,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
    QwtDoubleInterval interval)
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    const ViewSettings& vs = m_settings->GetViewSettings (viewNumber);
    const char* scalarName = FaceScalar::ToString (vs.GetBodyOrFaceScalar ());
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    const Foam& foam = m_average[viewNumber]->GetFoam ();
    ViewPipeline& pipeline = m_pipeline[viewNumber];
    G3D::AABox vv = CalculateViewingVolume (viewNumber);
    G3D::AABox bb = simulation.GetBoundingBox ();
    average->AverageInitStep ();
    int direction = 0;
    pipeline.UpdateAverage (average, direction);
    pipeline.UpdateFromOpenGl (vs, bb, foam);
    pipeline.UpdateOpacity (m_settings->GetContextAlpha ());
    pipeline.UpdateThreshold (interval);
    pipeline.UpdateColorTransferFunction (colorTransferFunction, scalarName);
    pipeline.UpdateTitle (m_settings->IsTitleShown (), average, viewNumber);
    pipeline.UpdateFocus (m_settings->GetViewNumber () == viewNumber);

    float w = width ();
    float h = height ();
    G3D::Rect2D vr = GetViewRect (viewNumber);
    G3D::Rect2D viewRect = G3D::Rect2D::xyxy (vr.x0 () / w, vr.y0 () / h,
					      vr.x1 () / w, vr.y1 () / h);
    renderWindow->AddRenderer(pipeline.m_renderer);
    pipeline.m_renderer->SetViewport (viewRect.x0 (), viewRect.y0 (),
				      viewRect.x1 (), viewRect.y1 ());
    resizeEvent (0);
    pipeline.m_renderer->ResetCamera ();
    setVisible (true);
}


void WidgetVtk::UpdateAverage (
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    ForAllViews (
	boost::bind (&WidgetVtk::updateViewAverage, this, _1, direction));
}
void WidgetVtk::updateViewAverage (
    ViewNumber::Enum viewNumber,
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    ViewPipeline& pipeline = m_pipeline[viewNumber];
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateAverage (average, direction[viewNumber]);
}


void WidgetVtk::UpdateTitle ()
{
    ForAllViews (
	boost::bind (&WidgetVtk::updateViewTitle, this, _1));
    update ();
}
void WidgetVtk::updateViewTitle (ViewNumber::Enum viewNumber)
{
    bool titleShown = m_settings->IsTitleShown ();
    ViewPipeline& pipeline = m_pipeline[viewNumber];
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateTitle (titleShown, average, viewNumber);
}


void WidgetVtk::UpdateFocus ()
{
    ForAllViews (
	boost::bind (&WidgetVtk::updateViewFocus, this, _1));
    update ();
}
void WidgetVtk::updateViewFocus (ViewNumber::Enum viewNumber)
{
    bool focus = m_settings->GetViewNumber () == viewNumber;
    ViewPipeline& pipeline = m_pipeline[viewNumber];
    pipeline.UpdateFocus (focus);
}



void WidgetVtk::ForAllViews (boost::function <void (ViewNumber::Enum)> f)
{
    for (int i = 0; i < m_settings->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (m_settings->IsVtkView (viewNumber))
	    f (viewNumber);
    }
}

G3D::AABox WidgetVtk::CalculateViewingVolume (ViewNumber::Enum viewNumber)
{
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = m_settings->GetVtkCount (&mapping);
    G3D::AABox vv = m_settings->CalculateViewingVolume (
	mapping[viewNumber], viewCount, simulation, width (), height ());
    return vv;
}

G3D::Rect2D WidgetVtk::GetViewRect (ViewNumber::Enum viewNumber)
{
    vector<ViewNumber::Enum> mapping;
    ViewCount::Enum viewCount = m_settings->GetVtkCount (&mapping);
    return m_settings->GetViewRect (
	width (), height (), mapping[viewNumber], viewCount);
}
