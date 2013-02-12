/**
 * @file   PipelineAverage3D.cpp
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */

#include "PipelineAverage3D.h"
#include "Body.h"
#include "DebugStream.h"
#include "Foam.h"
#include "RegularGridAverage.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"

//#define __LOG__(code) code
#define __LOG__(code)


// Methods PipelineAverage3D
// ======================================================================

PipelineAverage3D::PipelineAverage3D (
    size_t objects, size_t constraintSurfaces, size_t fontSize) :
    PipelineBase (fontSize, PipelineType::AVERAGE_3D)
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

    // scalar average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (threshold->GetOutputPort ());
    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    m_averageActor = averageActor;
    GetRenderer ()->AddViewProp(averageActor);

    // foam objects
    m_object.resize (objects);
    for (size_t i = 0; i < objects; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_object[i] = actor;
	GetRenderer ()->AddViewProp (actor);
    }

    // constraint faces rendered transparent
    m_constraintSurface.resize (constraintSurfaces);
    for (size_t i = 0; i < constraintSurfaces; ++i)
    {
	VTK_CREATE (vtkDataSetMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	m_constraintSurface[i] = actor;
	GetRenderer ()->AddViewProp (actor);
    }
}

void PipelineAverage3D::UpdateViewTitle (
    bool titleShown, const G3D::Vector2& position,
    const string& simulationName, const string& viewTitle)
{
    string title ("");
    ostringstream ostr;
    if (titleShown)
    {	
	ostr << simulationName << endl << viewTitle;
	title = ostr.str ();
    }
    PipelineBase::UpdateViewTitle (title.c_str (), position);
}


void PipelineAverage3D::UpdateThreshold (QwtDoubleInterval interval)
{
    if (m_threshold != 0)
    {
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
    }
}

void PipelineAverage3D::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char * name)
{
    if (m_averageActor != 0)
    {
        PipelineBase::UpdateColorTransferFunction (colorTransferFunction, name);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
    }
}

void PipelineAverage3D::UpdateOpacity (float contextAlpha)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
    {
	actor->GetProperty ()->SetOpacity (contextAlpha);
    }
}



void PipelineAverage3D::UpdateAverage (
    boost::shared_ptr<RegularGridAverage> average)
{
    const Foam& foam = average->GetFoam ();
    const ViewSettings& vs = average->GetViewSettings ();

    m_threshold->SetInputDataObject (average->GetAverage ());
    Foam::Bodies objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
	vtkDataSetMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInputDataObject (objects[i]->GetPolyData ());
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
	    ->SetInputDataObject (foam.GetConstraintFacesPolyData (p.first));
	++i;
    }
}
