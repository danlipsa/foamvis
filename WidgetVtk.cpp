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
     // add a renderer
    m_renderer = vtkRenderer::New();
    m_renderer->SetBackground(1,1,1);
    GetRenderWindow()->AddRenderer(m_renderer);

    m_mapper = vtkDataSetMapper::New ();
    m_actor = vtkActor::New();
    m_actor->SetMapper(m_mapper);
    m_renderer->AddViewProp(m_actor);
}

void WidgetVtk::UpdateAverage (const Foam& foam, 
			       BodyProperty::Enum bodyProperty)
{
    vtkSmartPointer<vtkUnstructuredGrid> aTetraGrid = 
	foam.SetCellScalar (foam.GetTetraGrid (), bodyProperty);
    m_mapper->SetInput (aTetraGrid);
    update ();
}
