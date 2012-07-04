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


// Methods
// ======================================================================

WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent)
{
}

void WidgetVtk::paintEvent (QPaintEvent * event)
{
    QVTKWidget::paintEvent (event);
    Q_EMIT Paint ();
}

void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    m_mapper->SetLookupTable (colorTransferFunction);    
}

void WidgetVtk::UpdateRenderStructured (
    const Foam& foam, const BodySelector& bodySelector,
    vtkSmartPointer<vtkMatrix4x4> modelView,
    BodyProperty::Enum bodyProperty)
{
    vtkSmartPointer<MeasureTimeVtk> measureTime (new MeasureTimeVtk ());

    // vtkUnstructuredGrid->vtkCellDatatoPointData, vtkImageData
    //           X                                      X 
    // ->vtkProbeFilter->vtkThreshold->
    //     
    // vtkDatasetMapper->vtkActor->vtkRenderer
    //       X               X
    vtkSmartPointer<vtkUnstructuredGrid> tetraFoamCell = 
	foam.SetCellScalar (foam.GetTetraGrid (bodySelector), bodySelector, 
			    bodyProperty);

    VTK_CREATE (vtkCellDataToPointData, cellToPoint);
    cellToPoint->SetInput (tetraFoamCell);

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
    measureTime->Measure (regularProbe);

    VTK_CREATE (vtkThreshold, threshold);
    threshold->ThresholdByUpper (0.1);
    threshold->AllScalarsOn ();
    threshold->SetInputConnection (regularProbe->GetOutputPort ());

    VTK_CREATE (vtkDataSetMapper, mapper);
    mapper->SetInputConnection (threshold->GetOutputPort ());    
    m_mapper = mapper;

    VTK_CREATE(vtkActor, actor);
    actor->SetMapper(mapper);
    actor->SetUserMatrix (modelView);

    VTK_CREATE (vtkRenderer, renderer);
    renderer->SetBackground(1,1,1);
    renderer->AddViewProp(actor);

    GetRenderWindow()->AddRenderer(renderer);
    update ();
}

