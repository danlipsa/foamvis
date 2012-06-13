/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "DebugStream.h"
#include "Foam.h"
#include "WidgetVtk.h"


WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent)
{
    // create a window to make it stereo capable and give it to QVTKWidget
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkRenderWindow::New ();

    // Activate 3DConnexion device
    SetUseTDx(true);    
    SetRenderWindow(renderWindow);
    
    const double angleSensitivity=0.02;
    const double translationSensitivity=0.001;
  
    QVTKInteractor *interactor=GetInteractor();
    vtkInteractorStyle *interactorStyle=
	static_cast<vtkInteractorStyle *>(interactor->GetInteractorStyle());
    vtkTDxInteractorStyleCamera *t= static_cast<vtkTDxInteractorStyleCamera *>(
	interactorStyle->GetTDxStyle());
  
    t->GetSettings()->SetAngleSensitivity(angleSensitivity);
    t->GetSettings()->SetTranslationXSensitivity(translationSensitivity);
    t->GetSettings()->SetTranslationYSensitivity(translationSensitivity);
    t->GetSettings()->SetTranslationZSensitivity(translationSensitivity);

     // add a renderer
    m_renderer = vtkRenderer::New();
    m_renderer->SetBackground(1,1,1);
    GetRenderWindow()->AddRenderer(m_renderer);
}

void WidgetVtk::UpdateAverage (const Foam& foam)
{
    vtkSmartPointer<vtkUnstructuredGrid> aTetraGrid = foam.GetTetraGrid ();
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkDataSetMapper::New ();
    mapper->SetInput(aTetraGrid);
    vtkSmartPointer<vtkActor> actor = vtkActor::New();
    actor->SetMapper(mapper);
    m_renderer->AddViewProp(actor);
    update ();
}
