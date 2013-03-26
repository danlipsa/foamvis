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
#include "Settings.h"
#include "Simulation.h"
#include "Utils.h"
#include "VectorOperation.h"
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
    createOutlineActor ();
}

void PipelineAverage3D::createOutlineActor ()
{
    VTK_CREATE (vtkOutlineFilter, outline);
    //outline.SetInputConnection(reader.GetOutputPort());
    VTK_CREATE (vtkPolyDataMapper, mapOutline);
    mapOutline->SetInputConnection(outline->GetOutputPort());
    VTK_CREATE (vtkActor, outlineActor);
    outlineActor->SetMapper(mapOutline);
    outlineActor->GetProperty()->SetColor(0, 0, 0);
    GetRenderer ()->AddViewProp(outlineActor);

    m_outline = outline;
    m_outlineActor = outlineActor;
}

void PipelineAverage3D::createScalarAverageActor ()
{
    // vtkImageData->vtkThreshold->vtkDatasetMapper->vtkActor->vtkRenderer
    //                                               vtkScalarBarActor

    // threshold scalar value
    VTK_CREATE (vtkThreshold, thresholdScalar);
    thresholdScalar->AllScalarsOn ();
    //thresholdScalar->SetInputDataObject ();

    // scalar average mapper and actor
    VTK_CREATE (vtkDataSetMapper, averageMapper);
    averageMapper->SetInputConnection (thresholdScalar->GetOutputPort ());
    VTK_CREATE(vtkActor, averageActor);
    averageActor->SetMapper(averageMapper);
    GetRenderer ()->AddViewProp(averageActor);

    m_scalarThreshold = thresholdScalar;
    m_scalarAverageActor = averageActor;
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
    //vtkPointSource          ->vtkProbeFilter->vtkThresholdPoints->vtkGlyph3D->
    //                              vtkPolyDataMapper->vtkActor
    //imageData->vtkThreshold->
     VTK_CREATE (vtkPointSource, seed);
    
    // remove invalid cells from the average
    // for the sphereall dataset, remove the voxels in the bounding box but 
    // outside the cilinder
    VTK_CREATE (vtkThreshold, thresholdOutsideCylinder);
    thresholdOutsideCylinder->AllScalarsOn ();
    thresholdOutsideCylinder->ThresholdByUpper (1.0);
    //thresholdOutsideCylinder->SetInputDataObject (imageData);

    VTK_CREATE (vtkProbeFilter, probe);
    probe->SetInputConnection (seed->GetOutputPort ());
    probe->SetSourceConnection (thresholdOutsideCylinder->GetOutputPort ());

    // remove invalid points
    // for the sphereall dataset, remove points outside the bounding box
    VTK_CREATE (vtkThresholdPoints, thresholdOutsideBB);
    thresholdOutsideBB->SetInputConnection (probe->GetOutputPort ());
    thresholdOutsideBB->ThresholdByUpper (1.0);

    // compute the velocity magnitude ...
    VTK_CREATE (vtkVectorNorm, norm);
    norm->SetInputConnection (thresholdOutsideBB->GetOutputPort());
    // and then select only points with with certain velocity magnitude
    VTK_CREATE (vtkThresholdPoints, thresholdNorm);
    thresholdNorm->SetInputConnection (norm->GetOutputPort ());
    //thresholdNorm->ThresholdByUpper (...);

    // the glyph
    VTK_CREATE (vtkArrowSource, arrow);

    // oriented and scaled glyph geometry at every point
    VTK_CREATE (vtkGlyph3D, glyph);
    glyph->SetSourceConnection(arrow->GetOutputPort());
    glyph->SetInputConnection(thresholdNorm->GetOutputPort());
    glyph->SetColorModeToColorByVector ();

    // mapper
    VTK_CREATE (vtkPolyDataMapper, mapper);
    mapper->SetInputConnection (glyph->GetOutputPort ());
    VTK_CREATE (vtkActor, actor);
    actor->SetMapper (mapper);
    GetRenderer ()->AddViewProp (actor);

    m_velocityGlyphSeeds = seed;    
    m_velocityGlyphThresholdOutsideCylinder = thresholdOutsideCylinder;
    m_velocityGlyphThresholdNorm = thresholdNorm;
    m_velocityGlyph = glyph;
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


void PipelineAverage3D::UpdateScalarThreshold (
    QwtDoubleInterval interval, BodyScalar::Enum scalar)
{
    if (m_scalarThreshold != 0)
    {
        //__ENABLE_LOGGING__;        
	m_scalarThreshold->ThresholdBetween (
	    interval.minValue (), interval.maxValue ());
        if (scalar == BodyScalar::VELOCITY_MAGNITUDE)
            m_velocityGlyphThresholdNorm->ThresholdBetween (
                interval.minValue (), interval.maxValue ());
        __LOG__ (cdbg << interval << endl;);
    }
}

void PipelineAverage3D::UpdateColorMap (
    const ColorBarModel& colorBarModel, const char * name)
{
    if (m_scalarAverageActor != 0)
    {
        vtkSmartPointer<vtkColorTransferFunction> vtkColorMap = 
            colorBarModel.GetVtkColorMap ();
        PipelineBase::UpdateColorMap (vtkColorMap, name);
	m_scalarAverageActor->GetMapper ()->SetLookupTable (
            vtkColorMap);
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

void PipelineAverage3D::UpdateOverlayMap (
    const ColorBarModel& colorBarModel, const char* name)
{
    vtkSmartPointer<vtkColorTransferFunction> vtkColorMap = 
        colorBarModel.GetVtkColorMap ();
    PipelineBase::UpdateOverlayMap (vtkColorMap, name);
    m_velocityGlyphActor->GetMapper ()->SetLookupTable (vtkColorMap);
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
    {
        vtkImageData* imageData = const_cast<vtkImageData*>(
            &velocityAverage.GetAverage ());
        m_velocityGlyphThresholdOutsideCylinder->SetInputDataObject (imageData);
    }
}


void PipelineAverage3D::UpdateScalarAverage (const RegularGridAverage& average)
{
    const Foam& foam = average.GetFoam ();
    const ViewSettings& vs = average.GetViewSettings ();
    vtkImageData* imageData = const_cast<vtkImageData*> (&average.GetAverage ());

    // update outline
    m_outline->SetInputDataObject (imageData);

    // update scalar
    m_scalarThreshold->SetInputDataObject (imageData);
    
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
    PipelineBase::FromView (viewNumber, base);
    const ViewSettings& vs = base.GetViewSettings (viewNumber);
    const Settings& settings = base.GetSettings ();
    updateAlpha (vs.GetContextAlpha (), m_constraintSurface);
    updateAlpha (vs.GetObjectAlpha (), m_object);
    m_scalarAverageActor->SetVisibility (vs.IsAverageShown ());
    m_outlineActor->SetVisibility (settings.AxesShown ());
    fromViewVelocityGlyph (viewNumber, base);
}


void PipelineAverage3D::fromViewVelocityGlyph (
    ViewNumber::Enum viewNumber, const Base& base)
{
    const ViewSettings& vs = base.GetViewSettings (viewNumber);
    m_velocityGlyphSeeds->SetNumberOfPoints (vs.GetGlyphSeedsCount ());

    G3D::AABox b = base.GetSimulation (viewNumber).GetBoundingBox ();
    G3D::Vector3 c = b.center ();
    m_velocityGlyphSeeds->SetCenter (c.x, c.y, c.z);
    m_velocityGlyphSeeds->SetRadius (b.extent ().max () / 2);

    m_velocityGlyphActor->SetVisibility (
        vs.IsVelocityShown () && vs.GetVelocityVis () == VectorVis::GLYPH);
    if (vs.IsVelocityGlyphSameSize ())
    {
        m_velocityGlyph->SetScaleModeToDataScalingOff ();
        m_velocityGlyph->SetScaleFactor (base.GetBubbleDiameter (viewNumber));
    }
    else
    {
        m_velocityGlyph->SetScaleModeToScaleByVector ();
        m_velocityGlyph->SetScaleFactor (base.GetBubbleDiameter (viewNumber));
        __LOG__ (cdbg << "scale factor: " 
                 << base.GetBubbleDiameter (viewNumber) << endl;);
        boost::shared_ptr<ColorBarModel> colorBarModel = 
            vs.GetOverlayBarModel ();
        if (colorBarModel != 0)
        {
            m_velocityGlyph->ClampingOn ();
            double range[2];
            range[0] = colorBarModel->GetClampMin ();
            range[1] = colorBarModel->GetClampMax ();
            m_velocityGlyph->SetRange (range);
            __LOG__ (cdbg << "range : " 
                     << range[0] << ", " << range[1] << endl;);
        }
    }
}

void PipelineAverage3D::updateAlpha (
    float alpha, vector<vtkSmartPointer<vtkActor> >& actors)
{
    BOOST_FOREACH (vtkSmartPointer<vtkActor> actor, actors)
	actor->GetProperty ()->SetOpacity (alpha);
}
