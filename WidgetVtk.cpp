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

void WidgetVtk::UpdateRenderUnstructured (
    const Foam& foam, const BodySelector& bodySelector,
    vtkSmartPointer<vtkMatrix4x4> modelView,
    BodyProperty::Enum bodyProperty,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    
    vtkSmartPointer<vtkUnstructuredGrid> aTetraGrid = 
	foam.SetCellScalar (foam.GetTetraGrid (bodySelector), bodySelector, 
			    bodyProperty);
    m_actor->SetUserMatrix (modelView);
    m_mapper->SetLookupTable (colorTransferFunction);
    m_mapper->SetInput (aTetraGrid);
    update ();
}

void WidgetVtk::UpdateRenderStructured (
    const Foam& foam, const BodySelector& bodySelector,
    vtkSmartPointer<vtkMatrix4x4> modelView,
    BodyProperty::Enum bodyProperty,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction)
{
    
    vtkSmartPointer<vtkUnstructuredGrid> tetraFoam = 
	foam.SetCellScalar (foam.GetTetraGrid (bodySelector), bodySelector, 
			    bodyProperty);
    VTK_CREATE (vtkImageData, regularFoam);
    size_t numberOfPoints = 256;
    G3D::AABox bb = foam.GetBoundingBox ();
    G3D::Vector3 low = bb.low ();
    G3D::Vector3 extent = bb.extent () / (numberOfPoints - 1);
    regularFoam->SetExtent (0, numberOfPoints - 1,
			    0, numberOfPoints - 1,
			    0, numberOfPoints - 1);
    regularFoam->SetOrigin (low.x, low.y, low.z);
    regularFoam->SetSpacing (extent.x, extent.y, extent.z);
    m_actor->SetUserMatrix (modelView);
    m_mapper->SetLookupTable (colorTransferFunction);
    m_mapper->SetInput (regularFoam);
    update ();
}
