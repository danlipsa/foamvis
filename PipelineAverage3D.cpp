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
    createScalarAverageActor ();
    createObjectActor (objectCount);
    createForceActor (objectCount);
    createConstraintSurfaceActor (constraintSurfaceCount);
    createVelocityGlyphActor ();
}

void PipelineAverage3D::createScalarAverageActor ()
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    //                                               vtkScalarBarActor

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
}

void PipelineAverage3D::createObjectActor (size_t objectCount)
{
    // foam objects
    m_object.resize (objectCount);
    setPolyActors (m_object.begin (), m_object.end ());    
}

void PipelineAverage3D::createForceActor (size_t objectCount)
{
    // (forces)          vtkArrowSource->vtkPolyDataMapper->vtkActor
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
}

void PipelineAverage3D::createConstraintSurfaceActor (
    size_t constraintSurfaceCount)
{
    // constraint faces rendered transparent
    // (constraint faces) vtkPolyData->vtkPolyDataMapper->vtkActor
    m_constraintSurface.resize (constraintSurfaceCount);
    setPolyActors (m_constraintSurface.begin (), m_constraintSurface.end ());    
}

void PipelineAverage3D::createVelocityGlyphActor ()
{
    // (velocity glyphs)  
    //vtkPointSource->vtkProbeFilter->vtkGlyph3D->vtkPolyDataMapper->vtkActor
    VTK_CREATE (vtkPointSource, seed);
    
    VTK_CREATE (vtkProbeFilter, probe);
    probe->SetInputConnection (seed->GetOutputPort ());
    //probe->SetSourceConnection (seed->GetOutputPort ());

    // velocity glyph is a cone
    VTK_CREATE (vtkConeSource, cone);
    cone->SetResolution (6);
    VTK_CREATE (vtkTransform, transform);
    transform->Translate (0.5, 0.0, 0.0);
    VTK_CREATE (vtkTransformPolyDataFilter, transformF);
    transformF->SetInputConnection (cone->GetOutputPort());
    transformF->SetTransform (transform);

    // oriented and scaled glyph geometry at every point
    VTK_CREATE (vtkGlyph3D, glyph);
    glyph->SetInputConnection(probe->GetOutputPort());
    glyph->SetSourceConnection(transformF->GetOutputPort());
    glyph->SetVectorModeToUseVector ();
    glyph->SetScaleModeToDataScalingOff ();
    //glyph->SetScaleModeToScaleByVector ();
    //glyph->SetScaleFactor (0.05);

    // mapper
    VTK_CREATE (vtkPolyDataMapper, mapper);
    mapper->SetInputConnection (glyph->GetOutputPort ());
    VTK_CREATE (vtkActor, actor);
    actor->SetMapper (mapper);
    actor->GetProperty ()->SetColor (0, 0, 0);
    GetRenderer ()->AddViewProp (actor);

    m_glyphSeeds = seed;
    m_glyphProbe = probe;
    m_velocityGlyphActor = actor;
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



void PipelineAverage3D::SetGlyphSeeds (const G3D::AABox& b)
{
    // update velocity glyphs
    G3D::Vector3 c = b.center ();
    m_glyphSeeds->SetCenter (c.x, c.y, c.z);
    m_glyphSeeds->SetRadius (b.extent ().min () / 2);
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
        //__ENABLE_LOGGING__;
	m_threshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
        __LOG__ (cdbg << interval << endl;);
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

void PipelineAverage3D::UpdateForceAverage (
    const ForceAverage& forceAverage)
{
    const Foam& foam = forceAverage.GetFoam ();
    const ViewSettings& vs = forceAverage.GetViewSettings ();
    const Foam::Bodies& objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
        G3D::Vector3 position = objects[i]->GetCenter ();
        __LOG__ (cdbg << "pos: " << position << endl;);
	if (vs.IsAverageAround ())
	    position += forceAverage.GetTranslation ();
        const ForceOneObject& forceOneObject = 
            forceAverage.GetAverageOneObject (i);
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
    float rotZDegrees = G3D::toDegrees (atan2 (force.y, force.x));
    // use the right hand rule
    float rotYDegrees = - G3D::toDegrees (
        atan2 (force.z, sqrt (force.y*force.y + force.x * force.x)));
    actor.SetOrientation (0, rotYDegrees, rotZDegrees);
    __LOG__ (cdbg << "force: " << forceType << 
             " rotY: " << rotYDegrees << " rotZ: " << rotZDegrees << endl;);
    actor.SetVisibility (shown);
}

void PipelineAverage3D::UpdateVelocityAverage (
    const RegularGridAverage& velocityAverage)
{
    const ViewSettings& vs = velocityAverage.GetViewSettings ();
    if (vs.IsVelocityShown () && vs.GetVelocityVis () == VectorVis::GLYPH)
        m_glyphProbe->SetSourceData (
            const_cast<vtkImageData*>(&velocityAverage.GetAverage ()));
}


void PipelineAverage3D::UpdateScalarAverage (const RegularGridAverage& average)
{
    const Foam& foam = average.GetFoam ();
    const ViewSettings& vs = average.GetViewSettings ();
    // update scalar
    m_threshold->SetInputDataObject (
        const_cast<vtkImageData*> (&average.GetAverage ()));


    // update objects
    const Foam::Bodies& objects = foam.GetObjects ();
    for (size_t i = 0; i < objects.size (); ++i)
    {
	vtkPolyDataMapper::SafeDownCast (m_object[i]->GetMapper ())
	    ->SetInputDataObject (objects[i]->GetPolyData ());
	if (vs.IsAverageAround ())
	{
	    G3D::Vector3 t = average.GetTranslation ();
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

void PipelineAverage3D::FromView (ViewNumber::Enum viewNumber, const Base& base)
{
    const ViewSettings& vs = base.GetViewSettings (viewNumber);

    updateAlpha (vs.GetContextAlpha (), m_constraintSurface);
    updateAlpha (vs.GetObjectAlpha (), m_object);
    m_averageActor->SetVisibility (vs.IsAverageShown ());    
    m_glyphSeeds->SetNumberOfPoints (vs.GetGlyphSeedsCount ());
    m_velocityGlyphActor->SetVisibility (
        vs.IsVelocityShown () && vs.GetVelocityVis () == VectorVis::GLYPH);
}

void PipelineAverage3D::updateAlpha (
    float alpha, vector<vtkSmartPointer<vtkActor> >& actors)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, actors)
	actor->GetProperty ()->SetOpacity (alpha);
}
