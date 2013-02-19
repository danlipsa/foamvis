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
#include "ViewSettings.h"


// Private Classes/Functions
// ======================================================================

typedef boost::function<void (vtkSmartPointer<vtkFloatArray>, 
			      vtkSmartPointer<vtkFloatArray>, 
			      double)> VectorOpScalarType;

/*
 * left = left op right
 */
struct VectorOpVector
{
    VectorOpVector (RegularGridAverage::OpType f) : 
	m_f (f)
    {
    }
    
    void operator() (vtkSmartPointer<vtkFloatArray> left,
		     vtkSmartPointer<vtkFloatArray> right)
    {
	size_t components = left->GetNumberOfComponents ();
	vtkIdType tuples = left->GetNumberOfTuples ();
	for (vtkIdType i = 0; i < tuples; ++i)
	{
	    double leftValue[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
	    double rightValue[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
	    left->GetTuple (i, leftValue);
	    right->GetTuple (i, rightValue);
	    for (size_t j = 0; j < components; ++j)
	    {
		leftValue[j] = m_f (leftValue[j], rightValue[j]);
	    }
	    left->SetTuple (i, leftValue);
	}
    }

    void operator () (G3D::Vector3& left, const G3D::Vector3& right)
    {
	for (size_t i = 0; i < 3; ++i)
	    left[i] = m_f (left[i], right[i]);
    }
    
private:
    RegularGridAverage::OpType m_f;
};

struct VectorOpScalar
{
    typedef boost::function<double (double, double)> OpType;

    VectorOpScalar (OpType f) : 
	m_f (f)
    {
    }
    
    void operator() (
	vtkSmartPointer<vtkFloatArray> left, 
	vtkSmartPointer<vtkFloatArray> right, double scalar)
    {
	size_t components = left->GetNumberOfComponents ();
	vtkIdType tuples = left->GetNumberOfTuples ();
	for (vtkIdType i = 0; i < tuples; ++i)
	{
	    double leftValue[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
	    double rightValue[BodyAttribute::MAX_NUMBER_OF_COMPONENTS];
	    right->GetTuple (i, rightValue);
	    for (size_t j = 0; j < components; ++j)
	    {
		leftValue[j] = m_f (rightValue[j], scalar);
	    }
	    left->SetTuple (i, leftValue);
	}
    }
    
private:
    OpType m_f;
};



// Methods
// ======================================================================

RegularGridAverage::RegularGridAverage (
    size_t bodyAttribute, ViewNumber::Enum viewNumber,
    const Settings& settings, const SimulationGroup& simulationGroup) :
    Average (viewNumber, settings, simulationGroup),
    m_bodyAttribute (bodyAttribute)
{
}

void RegularGridAverage::AverageInit ()
{
    Average::AverageInit ();
    const Simulation& simulation = GetSimulation ();
    size_t regularGridResolution = simulation.GetRegularGridResolution ();
    int extent[6] = {0, regularGridResolution -1,
                     0, regularGridResolution -1,
                     0, regularGridResolution -1};
    m_sum = CreateEmptyRegularGrid (GetBodyAttribute (), extent, 
                                    simulation.GetBoundingBox ());
    m_average = CreateEmptyRegularGrid (
	GetBodyAttribute (), extent, simulation.GetBoundingBox ());
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
    const char* attributeName = BodyAttribute::ToString (attribute);
    VectorOpVector vf (f);
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
    vtkSmartPointer<vtkFloatArray> sumAttribute = 
	vtkFloatArray::SafeDownCast (
	    m_sum->GetPointData ()->GetArray (attributeName));
    vtkSmartPointer<vtkFloatArray> newAttribute = 
	vtkFloatArray::SafeDownCast (
	    regularFoam->GetPointData ()->GetArray (attributeName));
    vf (sumAttribute, newAttribute);
    m_sum->Modified ();
}

void RegularGridAverage::computeAverage ()
{
    size_t attribute = GetBodyAttribute ();
    const char* attributeName = BodyAttribute::ToString (attribute);
    vtkSmartPointer<vtkFloatArray> averageAttribute = 
	vtkFloatArray::SafeDownCast (
	    m_average->GetPointData ()->GetArray (attributeName));
    vtkSmartPointer<vtkFloatArray> sumAttribute = 
	vtkFloatArray::SafeDownCast (
	    m_sum->GetPointData ()->GetArray (attributeName));
    VectorOpScalarType f = VectorOpScalar (std::divides<double> ()); 
    f (averageAttribute, sumAttribute, GetCurrentTimeWindow ());
    m_average->Modified ();
}

vtkSmartPointer<vtkImageData> RegularGridAverage::GetAverage ()
{
    if (m_average->GetMTime () < m_sum->GetMTime ())
	computeAverage ();
    return m_average;
}

G3D::Vector3 RegularGridAverage::GetTranslation (size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings ();
    const Simulation& simulation = GetSimulation ();
    G3D::AABox bb = simulation.GetBoundingBox ();
    G3D::Vector3 center = bb.center ();
    const ObjectPosition current = vs.GetAverageAroundPosition (timeStep);
    G3D::Vector3 t = center - current.m_rotationCenter;
    return t;
}

G3D::Vector3 RegularGridAverage::GetTranslation () const
{
    const ViewSettings& vs = GetViewSettings ();
    return GetTranslation (vs.GetTime ());
}
