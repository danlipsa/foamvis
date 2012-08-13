/**
 * @file   RegularGridAverage.cpp
 * @author Dan R. Lipsa
 * @date  24 July 2010
 *
 * Implementation for the RegularGridAverage class 
 *
 */

#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"

//#define __LOG__(code) code
#define __LOG__(code)


// Private Classes/Functions
// ======================================================================

struct VectorOpVector
{
    typedef boost::function<double (double, double)> OpType;

    VectorOpVector (OpType f) : 
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
    
private:
    OpType m_f;
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

RegularGridAverage::RegularGridAverage (ViewNumber::Enum viewNumber,
					const Settings& settings, 
					const SimulationGroup& simulationGroup) :
    Average (viewNumber, settings, simulationGroup)
{
}


void RegularGridAverage::AverageInit ()
{
    Average::AverageInit ();
    const Simulation& simulation = GetSimulation ();
    size_t regularGridResolution = simulation.GetRegularGridResolution ();
    m_sum = CreateEmptyRegularGrid (
	GetBodyAttribute (), regularGridResolution, 
	simulation.GetBoundingBox ());
    m_average = CreateEmptyRegularGrid (
	GetBodyAttribute (), regularGridResolution, 
	simulation.GetBoundingBox ());
    __LOG__ (cdbg << "RegularGridAverage::AverageInit: " << viewNumber << endl;)
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
    opStep (timeStep, VectorOpVector (std::plus<double> ()));
    __LOG__ (cdbg << "addStep" << endl;)
}

void RegularGridAverage::removeStep (size_t timeStep, size_t subStep)
{
    (void)subStep;
    opStep (timeStep, VectorOpVector (std::minus<double> ()));
    __LOG__ (cdbg << "removeStep" << endl;)
}


void RegularGridAverage::opStep (
    size_t timeStep, RegularGridAverage::VectorOpVectorType f)
{
    const Foam& foam = GetFoam (timeStep);
    const ViewSettings& vs = GetViewSettings ();
    size_t attribute = GetBodyAttribute ();
    vtkSmartPointer<vtkImageData> regularFoam = foam.GetRegularGrid (attribute);
    const char* attributeName = BodyAttribute::ToString (attribute);
    if (vs.IsAverageAround ())
    {
	const ObjectPosition current = 
	    vs.GetAverageAroundPosition (vs.GetCurrentTime ());
    }
    vtkSmartPointer<vtkFloatArray> sumAttribute = 
	vtkFloatArray::SafeDownCast (
	    m_sum->GetPointData ()->GetArray (attributeName));
    vtkSmartPointer<vtkFloatArray> newAttribute = 
	vtkFloatArray::SafeDownCast (
	    regularFoam->GetPointData ()->GetArray (attributeName));
    f (sumAttribute, newAttribute);
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
