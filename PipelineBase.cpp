/**
 * @file   PipelineBase.cpp
 * @author Dan R. Lipsa
 * @date 24 Sept 2012
 * 
 * VTK pipeline
 * 
 */

#include "PipelineBase.h"
#include "Utils.h"

PipelineBase::PipelineBase (size_t fontSize)
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
    mapper->SetInput (Grid);
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

void PipelineBase::UpdateScalarBar (
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
