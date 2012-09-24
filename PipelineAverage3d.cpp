/**
 * @file   PipelineAverage3d.cpp
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */

#include "PipelineAverage3d.h"
#include "Body.h"
#include "DebugStream.h"
#include "Foam.h"
#include "RegularGridAverage.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"

//#define __LOG__(code) code
#define __LOG__(code)


// Methods PipelineAverage3d
// ======================================================================

PipelineAverage3d::PipelineAverage3d (
    size_t objects, size_t constraintSurfaces, size_t fontSize) :
    PipelineBase (fontSize)
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

void PipelineAverage3d::UpdateViewTitle (
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
    PipelineBase::UpdateViewTitle (title.c_str (), position);
}


void PipelineAverage3d::UpdateThreshold (QwtDoubleInterval interval)
{
    if (m_threshold != 0)
    {
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
    }
}

void PipelineAverage3d::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char * name)
{
    if (m_averageActor != 0)
    {
        UpdateScalarBar (colorTransferFunction, name);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
    }
}

void PipelineAverage3d::UpdateOpacity (float contextAlpha)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, m_constraintSurface)
    {
	actor->GetProperty ()->SetOpacity (contextAlpha);
    }
}

void PipelineAverage3d::ViewToVtk (
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

void PipelineAverage3d::VtkToView (
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


void PipelineAverage3d::UpdateAverage (
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
