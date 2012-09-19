/**
 * @file   Average3dPipeline.cpp
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */

#include "Average3dPipeline.h"
#include "Body.h"
#include "DebugStream.h"
#include "Foam.h"
#include "RegularGridAverage.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"

//#define __LOG__(code) code
#define __LOG__(code)


// Methods Average3dPipeline
// ======================================================================

Average3dPipeline::Average3dPipeline (
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    //                                      vtkScalarBarActor 
    // 
    // (foam objects)    vtkPolyData->vtkDatasetMapper->vtkActor
    // (constraint faces)

    createRenderer ();
    createScalarBar ();
    createViewTitle (fontSize);
    createFocusRect ();

    // threshold
    VTK_CREATE (vtkThreshold, threshold);
    threshold->AllScalarsOn ();
    m_threshold = threshold;

    // scalar average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (threshold->GetOutputPort ());
    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    m_averageActor = averageActor;
    m_renderer->AddViewProp(averageActor);

    // foam objects
    m_object.resize (objects);
    for (size_t i = 0; i < objects; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_object[i] = actor;
	m_renderer->AddViewProp (actor);
    }

    // constraint faces rendered transparent
    m_constraintSurface.resize (constraintSurfaces);
    for (size_t i = 0; i < constraintSurfaces; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_constraintSurface[i] = actor;
	m_renderer->AddViewProp (actor);
    }
}

void Average3dPipeline::createRenderer ()
{
    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);
    renderer->LightFollowCameraOn ();
    m_renderer = renderer;
}

void Average3dPipeline::createScalarBar ()
{
    VTK_CREATE (vtkScalarBarActor, scalarBar);
    scalarBar->SetOrientationToVertical ();
    scalarBar->SetNumberOfLabels (3);
    m_scalarBar = scalarBar;
    m_renderer->AddViewProp (scalarBar);
}

void Average3dPipeline::createViewTitle (size_t fontSize)
{
    VTK_CREATE (vtkTextProperty, singleLineTextProp);
    singleLineTextProp->SetFontSize (fontSize);
    singleLineTextProp->SetFontFamilyToArial ();
    singleLineTextProp->BoldOff ();
    singleLineTextProp->ItalicOff ();
    singleLineTextProp->ShadowOff ();
    
    VTK_CREATE(vtkTextProperty, multiLineTextProp);
    multiLineTextProp->ShallowCopy (singleLineTextProp);
    multiLineTextProp->ShadowOn ();
    multiLineTextProp->SetLineSpacing (1.2);

    VTK_CREATE(vtkTextMapper, textMapper);
    vtkTextProperty* tprop = textMapper->GetTextProperty ();
    tprop->ShallowCopy (multiLineTextProp);
    tprop->SetJustificationToCentered ();
    tprop->SetVerticalJustificationToTop ();
    tprop->SetColor (0, 0, 0);
    
    VTK_CREATE (vtkActor2D, textActor);
    textActor->SetMapper (textMapper);
    textActor->GetPositionCoordinate ()->
        SetCoordinateSystemToNormalizedDisplay ();
    m_renderer->AddViewProp (textActor);
    m_viewTitleActor = textActor;
}

void Average3dPipeline::createFocusRect ()
{
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
}


void Average3dPipeline::UpdateViewTitle (
    bool titleShown, const G3D::Vector2& position,
    boost::shared_ptr<RegularGridAverage> average, ViewNumber::Enum viewNumber)
{
    string title ("");
    ostringstream ostr;
    if (titleShown)
    {	
	ostr << average->GetSimulation ().GetName () << endl 
	     << average->GetViewSettings ().GetTitle (viewNumber);
	title = ostr.str ();
    }
    vtkTextMapper::SafeDownCast (m_viewTitleActor->GetMapper ())->SetInput (
        title.c_str ());
    m_viewTitleActor->GetPositionCoordinate ()->SetValue (position.x, position.y);
}


void Average3dPipeline::UpdateThreshold (QwtDoubleInterval interval)
{
    if (m_threshold != 0)
    {
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
    }
}

void Average3dPipeline::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char * name)
{
    if (m_averageActor != 0)
    {
	m_scalarBar->SetLookupTable (colorTransferFunction);
        (void)name;
	//m_scalarBar->SetTitle (name);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
    }
}


void Average3dPipeline::PositionScalarBar (G3D::Rect2D position)
{
    m_scalarBar->SetHeight (position.height ());
    m_scalarBar->SetWidth (position.width ());
    m_scalarBar->SetPosition (position.x0 (), position.y0 ());
}

void Average3dPipeline::UpdateOpacity (float contextAlpha)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
    {
	actor->GetProperty ()->SetOpacity (contextAlpha);
    }
}

void Average3dPipeline::UpdateFocus (bool focus)
{
    if (focus)
	m_renderer->AddViewProp (m_focusActor);
    else
	m_renderer->RemoveViewProp (m_focusActor);
}


void Average3dPipeline::ViewToVtk (
    const ViewSettings& vs, G3D::Vector3 center, const Foam& foam)
{
    G3D::Matrix3 cameraRotationAxes = 
        vs.GetRotationForAxesOrder (foam).inverse ();
    G3D::Matrix3 cameraRotation = vs.GetRotation ().inverse ();

    G3D::Vector3 rotationCenter = vs.GetRotationCenter ();
    G3D::Vector3 up = G3D::Vector3 (0, 1, 0);
    G3D::Vector3 position = center + G3D::Vector3 (0, 0, 1);
    // apply the rotations from ModelViewTransform in reverse order
    // rotation around the rotationCenter
    up = cameraRotation * up;
    position = cameraRotation * (position - rotationCenter) + rotationCenter;
    center = cameraRotation * (center - rotationCenter) + rotationCenter;

    // rotation around center
    up = cameraRotationAxes * up ;
    position = cameraRotationAxes * (position - center) + center;

    vtkCamera* camera = m_renderer->GetActiveCamera ();
    camera->SetFocalPoint (center.x, center.y, center.z);
    camera->SetPosition (position.x, position.y, position.z);
    camera->ComputeViewPlaneNormal ();
    camera->SetViewUp (up.x, up.y, up.z);
    m_renderer->ResetCamera ();
}

void Average3dPipeline::VtkToView (
    ViewSettings& vs, const Foam& foam)
{
    vtkCamera* camera = m_renderer->GetActiveCamera ();
    double center[3];
    double position[3];
    double up[3];
    camera->GetFocalPoint (center);
    camera->GetPosition (position);
    camera->GetViewUp (up);

    G3D::Vector3 one (up[0], up[1], up[2]);
    G3D::Vector3 two = G3D::Vector3 (position[0] - center[0],
                                     position[1] - center[1],
                                     position[2] - center[2]).unit ();
    G3D::Vector3 three = one.cross (two);
    G3D::Matrix3 m = MatrixFromColumns (one, two, three);
    G3D::Matrix3 mInitial = MatrixFromColumns (G3D::Vector3 (0, 1, 0),
                                               G3D::Vector3 (0, 0, 1),
                                               G3D::Vector3 (1, 0, 0));
    G3D::Matrix3 cRAm = vs.GetRotationForAxesOrder (foam);
    G3D::Matrix3 rCamera =  cRAm * m * mInitial.inverse ();
    vs.SetRotation (rCamera.inverse ());
    if (vs.GetRotationCenterType () != ViewSettings::ROTATION_CENTER_FOAM)
    {
        vs.SetRotationCenterType (ViewSettings::ROTATION_CENTER_FOAM);
        vs.SetRotationCenter (G3D::Vector3 (center[0], center[1], center[2]));
    }
}


void Average3dPipeline::UpdateAverage (
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
