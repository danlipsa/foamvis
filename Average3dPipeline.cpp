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
    renderer->AddViewProp (textActor);
    m_textActor = textActor;

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
}

void Average3dPipeline::UpdateTitle (
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
    vtkTextMapper::SafeDownCast (m_textActor->GetMapper ())->SetInput (
        title.c_str ());
    m_textActor->GetPositionCoordinate ()->SetValue (position.x, position.y);
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
	m_scalarBar->SetTitle (name);
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


void Average3dPipeline::CopyTransformationFromView (
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
