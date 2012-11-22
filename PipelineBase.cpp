/**
 * @file   PipelineBase.cpp
 * @author Dan R. Lipsa
 * @date 24 Sept 2012
 * 
 * VTK pipeline
 * 
 */

#include "Foam.h"
#include "PipelineBase.h"
#include "Utils.h"
#include "ViewSettings.h"

PipelineBase::PipelineBase (size_t fontSize, PipelineType::Enum type) :
    m_type (type)
{
    createRenderer ();
    createScalarBar ();
    createViewTitle (fontSize);
    createFocusRect ();
}


void PipelineBase::createRenderer ()
{
    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);
    renderer->LightFollowCameraOn ();
    m_renderer = renderer;
}

void PipelineBase::createScalarBar ()
{
    VTK_CREATE (vtkScalarBarActor, scalarBar);
    scalarBar->SetOrientationToVertical ();
    scalarBar->SetNumberOfLabels (3);
    m_scalarBar = scalarBar;
    m_renderer->AddViewProp (scalarBar);
}

void PipelineBase::createViewTitle (size_t fontSize)
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

void PipelineBase::createFocusRect ()
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
    mapper->SetInputDataObject (Grid);
    mapper->SetTransformCoordinate (normCoords);
    
    VTK_CREATE (vtkActor2D, focusActor);
    focusActor->SetMapper (mapper);
    focusActor->GetProperty ()->SetColor (0.1, 0.1, 0.1);
    m_focusRectActor = focusActor;
}

void PipelineBase::UpdateViewTitle (
    const char* title, const G3D::Vector2& position)
{
    vtkTextMapper::SafeDownCast (m_viewTitleActor->GetMapper ())->SetInput (
        title);
    m_viewTitleActor->GetPositionCoordinate ()->SetValue (
        position.x, position.y);
}

void PipelineBase::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char * name)
{
    m_scalarBar->SetLookupTable (colorTransferFunction);
    (void)name;
    //m_scalarBar->SetTitle (name);
}

void PipelineBase::PositionScalarBar (G3D::Rect2D position)
{
    m_scalarBar->SetHeight (position.height ());
    m_scalarBar->SetWidth (position.width ());
    m_scalarBar->SetPosition (position.x0 (), position.y0 ());
}

void PipelineBase::UpdateFocus (bool focus)
{
    if (focus)
	GetRenderer ()->AddViewProp (m_focusRectActor);
    else
	GetRenderer ()->RemoveViewProp (m_focusRectActor);
}

void PipelineBase::ViewToVtk (
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

    vtkCamera* camera = GetRenderer ()->GetActiveCamera ();
    camera->SetFocalPoint (center.x, center.y, center.z);
    camera->SetPosition (position.x, position.y, position.z);
    camera->ComputeViewPlaneNormal ();
    camera->SetViewUp (up.x, up.y, up.z);
    GetRenderer ()->ResetCamera ();
}

void PipelineBase::VtkToView (
    ViewSettings& vs, const Foam& foam)
{
    vtkCamera* camera = GetRenderer ()->GetActiveCamera ();
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
