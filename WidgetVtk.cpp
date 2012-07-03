/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "BodySelector.h"
#include "DebugStream.h"
#include "Foam.h"
#include "WidgetVtk.h"


WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent)
{
     // add a renderer
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(1,1,1);
    GetRenderWindow()->AddRenderer(m_renderer);
    m_mapper = vtkSmartPointer<vtkDataSetMapper>::New ();
    
    m_actor = vtkSmartPointer<vtkActor>::New ();    
    m_actor->SetMapper(m_mapper);
    m_renderer->AddViewProp(m_actor);
}

void WidgetVtk::UpdateRenderStructured (
    const Foam& foam, const BodySelector& bodySelector,
    vtkSmartPointer<vtkMatrix4x4> modelView,
    BodyProperty::Enum bodyProperty,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    // vtkUnstructuredGrid->
    // vtkCellDatatoPointData, vtkImageData ->vtkProbeFilter->vtkShrinkFilter->vtkDatasetMapper
    // vtkImageData          ->
    vtkSmartPointer<vtkUnstructuredGrid> tetraFoam = 
	foam.SetCellScalar (foam.GetTetraGrid (bodySelector), bodySelector, 
			    bodyProperty);
    VTK_CREATE (vtkCellDataToPointData, cellToPoint);
    cellToPoint->SetInput (tetraFoam);

    size_t pointsPerAxis = 64;
    G3D::AABox bb = foam.GetBoundingBox ();
    G3D::Vector3 spacing = bb.extent () / (pointsPerAxis - 1);
    G3D::Vector3 origin = bb.low ();
    VTK_CREATE (vtkImageData, regularFoam);
    regularFoam->SetExtent (0, pointsPerAxis - 1,
			    0, pointsPerAxis - 1,
			    0, pointsPerAxis - 1);
    regularFoam->SetOrigin (origin.x, origin.y, origin.z);
    regularFoam->SetSpacing (spacing.x, spacing.y, spacing.z);

    VTK_CREATE (vtkProbeFilter, regularProbe);
    regularProbe->SetSourceConnection (cellToPoint->GetOutputPort ());
    regularProbe->SetInput (regularFoam);

    VTK_CREATE (vtkThreshold, threshold);
    threshold->ThresholdByUpper (0.1);
    threshold->AllScalarsOn ();
    threshold->SetInputConnection (regularProbe->GetOutputPort ());

    // VTK_CREATE (vtkShrinkFilter, shrink);
    // shrink->SetInputConnection (threshold->GetOutputPort ());
    // shrink->SetShrinkFactor (0.5);

    m_actor->SetUserMatrix (modelView);
    m_mapper->SetLookupTable (colorTransferFunction);
    m_mapper->SetInputConnection (threshold->GetOutputPort ());
    update ();
}

