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
#include "ColorBarModel.h"
#include "Debug.h"
#include "Foam.h"
#include "ForceAverage.h"
#include "RegularGridAverage.h"
#include "Simulation.h"
#include "Utils.h"
#include "ViewSettings.h"



// Methods PipelineAverage3D
// ======================================================================

PipelineAverage3D::PipelineAverage3D (
    size_t objectCount, size_t constraintSurfaceCount, size_t fontSize) :
    PipelineBase (fontSize, PipelineType::AVERAGE_3D)
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    //                                      vtkScalarBarActor 
    // 
    // (foam objects)    vtkPolyData->vtkPolyDataMapper->vtkActor
    // (constraint faces)
    // (forces)          vtkArrowSource->vtkPolyDataMapper->vtkActor

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
    m_object.resize (objectCount);
    setPolyActors (m_object.begin (), m_object.end ());

    // forces acting on objects
    m_forceActor.resize (objectCount);
    VTK_CREATE (vtkArrowSource, arrow);
    for (size_t i = 0; i < m_forceActor.size (); ++i)
    {
        for (size_t j = 0; j < m_forceActor[i].size (); ++j)
        {
            VTK_CREATE (vtkPolyDataMapper, mapper);
            mapper->SetInputConnection (arrow->GetOutputPort ());

            VTK_CREATE (vtkActor, actor);
            actor->SetMapper (mapper);
            GetRenderer ()->AddViewProp (actor);
            m_forceActor[i][j] = actor;
        }
    }

    // constraint faces rendered transparent
    m_constraintSurface.resize (constraintSurfaceCount);
    setPolyActors (m_constraintSurface.begin (), m_constraintSurface.end ());
}

template <typename Iterator>
void PipelineAverage3D::setPolyActors (Iterator begin, Iterator end)
{
    for (Iterator it = begin; it != end; ++it)
    {
	VTK_CREATE (vtkPolyDataMapper, mapper);

	VTK_CREATE (vtkActor, actor);
	actor->SetMapper (mapper);
	GetRenderer ()->AddViewProp (actor);
	*it = actor;
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

void PipelineAverage3D::UpdateColorBarModel (
    const ColorBarModel& colorBarModel, const char * name)
{
    if (m_averageActor != 0)
    {
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = 
            colorBarModel.GetColorTransferFunction ();
        PipelineBase::UpdateColorBarModel (colorTransferFunction, name);
	m_averageActor->GetMapper ()->SetLookupTable (colorTransferFunction);
    }
    for (size_t i = 0; i < m_forceActor.size (); ++i)
        for (size_t j = 0; j < m_forceActor[i].size (); ++j)
        {
            QColor color = colorBarModel.GetHighlightColor (
                HighlightNumber::Enum (j));
            m_forceActor[i][j]->GetProperty ()->SetColor (
                color.redF (), color.greenF (), color.blueF ());
        }
}

void PipelineAverage3D::UpdateContextAlpha (float alpha)
{
    updateAlpha (alpha, m_constraintSurface);
}

void PipelineAverage3D::UpdateObjectAlpha (float alpha)
{
    updateAlpha (alpha, m_object);
}

void PipelineAverage3D::updateAlpha (
    float alpha, vector<vtkSmartPointer<vtkActor> >& actors)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, actors)
	actor->GetProperty ()->SetOpacity (alpha);
}


void PipelineAverage3D::UpdateForceAverage (
    const ForceAverage& forceAverage)
{
    const Foam& foam = forceAverage.GetFoam ();
    const ViewSettings& vs = forceAverage.GetViewSettings ();
    const Foam::Bodies& objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
        G3D::Vector3 position = objects[i]->GetCenter ();
	if (vs.IsAverageAround ())
	    position += forceAverage.GetTranslation ();
        const ForceOneObject& forceOneObject = 
            forceAverage.GetForceOneObject (i);
        for (size_t j = ForceType::NETWORK; j <= ForceType::RESULT; ++j)
        {
            ForceType::Enum ft = ForceType::Enum (j);
            updateForce (i, ft, forceOneObject.GetForce (ft), position,
                         vs.IsForceShown (ft));
        }
    }
}

void PipelineAverage3D::updateForce (
    size_t objectIndex, ForceType::Enum forceType,
    G3D::Vector3 force, G3D::Vector3 position, bool shown)
{
    vtkActor& actor = *m_forceActor[objectIndex][forceType];
    actor.SetPosition (position.x, position.y, position.z);
    actor.SetScale (force.length ());
    float rotZRad = atan2 (force.y, force.x);
    float rotYRad = atan2 (force.z, sqrt (force.y*force.y + force.x * force.x));
    actor.SetOrientation (
        0, G3D::toDegrees (rotYRad), G3D::toDegrees (rotZRad));
    actor.SetVisibility (shown);
}

void PipelineAverage3D::UpdateScalarAverage (
    boost::shared_ptr<RegularGridAverage> average)
{
    const Foam& foam = average->GetFoam ();
    const ViewSettings& vs = average->GetViewSettings ();

    m_threshold->SetInputDataObject (average->GetAverage ());

    // update objects
    const Foam::Bodies& objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
	vtkPolyDataMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInputDataObject (objects[i]->GetPolyData ());
	if (vs.IsAverageAround ())
	{
	    G3D::Vector3 t = average->GetTranslation ();
	    m_object[i]->SetPosition (t.x, t.y, t.z);
	}
    }

    // update constraint surfaces
    size_t i = 0;
    pair<size_t, Foam::OrientedFaces> p;
    BOOST_FOREACH (p, foam.GetConstraintFaces ())
    {
	vtkPolyDataMapper::SafeDownCast (m_constraintSurface[i]->GetMapper ())
	    ->SetInputDataObject (foam.GetConstraintFacesPolyData (p.first));
	++i;
    }
}
