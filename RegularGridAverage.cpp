/**
 * @file   RegularGridAverage.cpp
 * @author Dan R. Lipsa
 * @date  24 July 2010
 *
 * Implementation for the RegularGridAverage class 
 *
 */

#include "Debug.h"
#include "Foam.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
#include "VectorOperation.h"
#include "ViewSettings.h"


// Private Classes/Functions
// ======================================================================




// Methods
// ======================================================================

RegularGridAverage::RegularGridAverage (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<Settings> settings, 
    boost::shared_ptr<const SimulationGroup> simulationGroup) :

    Average (viewNumber, settings, simulationGroup),
    m_bodyAttribute (BodyAttribute::COUNT)
{
}

void RegularGridAverage::AverageInit ()
{
    __ENABLE_LOGGING__;
    Average::AverageInit ();
    const Simulation& simulation = GetSimulation ();
    size_t regularGridResolution = simulation.GetRegularGridResolution ();
    int extent[6] = {0, regularGridResolution -1,
                     0, regularGridResolution -1,
                     0, regularGridResolution -1};
    m_sum = CreateEmptyRegularGrid (GetBodyAttribute (), extent, 
                                    simulation.GetBoundingBox ());
    AddValidPointMask (m_sum);
    m_average = CreateEmptyRegularGrid (
	GetBodyAttribute (), extent, simulation.GetBoundingBox ());
    AddValidPointMask (m_average);
    if (GetBodyAttribute () == BodyAttribute::VELOCITY)
        m_average->GetPointData ()->SetActiveScalars (
            VectorOperation::VALID_NAME);
    __LOG__ (
        int index;
        m_average->GetPointData ()->GetArray (
            VectorOperation::VALID_NAME, index);
        cdbg << "attribute: " << GetBodyAttribute () 
        << " VALID index: " << index << endl;);
}

void RegularGridAverage::AverageRelease ()
{
    m_sum = 0;
}


void RegularGridAverage::AverageRotateAndDisplay (
    StatisticsType::Enum displayType, G3D::Vector2 rotationCenter, 
    float angleDegrees) const
{
    (void)displayType;(void)rotationCenter;(void)angleDegrees;
    ThrowException ("AverageRotateAndDisplay not implemented");
}

void RegularGridAverage::addStep (
    size_t timeStep, size_t subStep)
{
    (void)subStep;
    opStep (timeStep, std::plus<double> ());
    __LOG__ (cdbg << "addStep" << endl;)
}

void RegularGridAverage::removeStep (size_t timeStep, size_t subStep)
{
    (void)subStep;
    opStep (timeStep, std::minus<double> ());
    __LOG__ (cdbg << "removeStep" << endl;)
}

void RegularGridAverage::opStep (size_t timeStep, RegularGridAverage::OpType f)
{
    const Foam& foam = GetFoam (timeStep);
    const ViewSettings& vs = GetViewSettings ();
    size_t attribute = GetBodyAttribute ();
    vtkSmartPointer<vtkImageData> regularFoam = foam.GetRegularGrid (attribute);
    if (vs.IsAverageAround ())
    {
	const Simulation& simulation = GetSimulation ();
	G3D::Vector3 translate = GetTranslation (timeStep);
	G3D::Vector3 origin (regularFoam->GetOrigin ());
	origin += translate;
	regularFoam->SetOrigin (origin.x, origin.y, origin.z);
        size_t regularGridResolution = simulation.GetRegularGridResolution ();
        int extent[6] = {0, regularGridResolution -1,
                         0, regularGridResolution -1,
                         0, regularGridResolution -1};
	vtkSmartPointer<vtkImageData> translatedRegularFoam = 
	    CreateEmptyRegularGrid (
		GetBodyAttribute (), extent, 
		simulation.GetBoundingBox ());
	VTK_CREATE (vtkProbeFilter, translatedDataProbe);
	translatedDataProbe->SetSourceData (regularFoam);
	translatedDataProbe->SetInputDataObject (translatedRegularFoam);
	translatedDataProbe->Update ();
	regularFoam = 
	    vtkImageData::SafeDownCast(translatedDataProbe->GetOutput ());
    }
    ImageOpImage (m_sum, regularFoam, f, attribute);
}

void RegularGridAverage::ComputeAverage ()
{
    if (m_average->GetMTime () < m_sum->GetMTime ())
        ImageOpScalar (m_average, m_sum, GetCurrentTimeWindow (),
                       std::divides<double> (), GetBodyAttribute ());
}

