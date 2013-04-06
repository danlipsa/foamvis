/**
 * @file Simulation.h
 * @author Dan R. Lipsa
 *
 * Declaration of Simulation object
 */
#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include "BodyAlongTime.h"
#include "Comparisons.h"
#include "DataProperties.h"
#include "HistogramStatistics.h"
#include "ObjectPosition.h"
#include "ForceOneObject.h"
#include "Utils.h"

class Foam;
class OOBox;
class Settings;

/**
 * Stores information about a list of DMP files
 */
class Simulation
{
public:
    typedef vector< boost::shared_ptr<Foam> > Foams;
    typedef void (Foam::*FoamMethod) ();
    typedef boost::function<void (Foam*)> FoamParamMethod;

public:
    Simulation ();
    void CacheBodiesAlongTime ();
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Foam objects
     */
    void CalculateBoundingBox ();
    bool Is2D () const;
    bool Is3D () const
    {
	return ! Is2D ();
    }
    bool IsTorus () const;

    /**
     * Gets the AABox for this vector of Foam objects
     */
    const G3D::AABox& GetBoundingBox () const
    {
	return m_boundingBox;
    }
    const G3D::AABox& GetBoundingBoxTorus () const
    {
	return m_boundingBoxTorus;
    }

    BodiesAlongTime& GetBodiesAlongTime ()
    {
	return m_bodiesAlongTime;
    }
    const BodiesAlongTime& GetBodiesAlongTime () const
    {
	return m_bodiesAlongTime;
    }
    const BodyAlongTime& GetBodyAlongTime (size_t id) const;
    
    const Body& GetBody (size_t bodyId, size_t timeStep) const;
    /**
     * Gets the vector of Foam objects
     * @return vector of data objects
     */
    Foam& GetFoam (size_t timeStep)
    {
	return *m_foams[timeStep];
    }

    const Foam& GetFoam (size_t timeStep) const
    {
	return *m_foams[timeStep];
    }

    Foams& GetFoams ()
    {
	return m_foams;
    }
    const Foams& GetFoams () const
    {
	return m_foams;
    }
    bool IsQuadratic () const;


    const HistogramStatistics& GetHistogram (size_t property) const
    {
	return m_histogram[property];
    }

    double GetMin (BodyScalar::Enum property) const
    {
	return acc::min (GetHistogram (property));
    }

    double GetMax (BodyScalar::Enum property) const
    {
	return acc::max (GetHistogram (property));
    }
    float GetMaxDeformationEigenValue () const
    {
	return m_maxDeformationEigenValue;
    }
    QwtDoubleInterval GetRange (BodyScalar::Enum property) const
    {
	return QwtDoubleInterval (GetMin (property), GetMax (property));
    }


    /**
     * For the histogram associated with each foam this is the maximum of 
     * GetMaxCountPerBin for all foams.
     */
    size_t GetMaxCountPerBinIndividual (BodyScalar::Enum property) const;

    size_t GetTimeSteps () const
    {
	return m_foams.size ();
    }
    /**
     * Returns the time steps for which the range of values is in one of the 
     * valueIntervals.
     */
    void GetTimeStepSelection (
	BodyScalar::Enum property,
	const vector<QwtDoubleInterval>& valueIntervals,
	vector<bool>* timeStepSelection) const;
    void GetTimeStepSelection (
	BodyScalar::Enum property,
	const QwtDoubleInterval& valueInterval,
	vector<bool>* timeStepSelection) const;

    void Preprocess ();

    void SetName (string simulationName)
    {
	m_name = simulationName;
    }
    string GetName () const
    {
	return m_name;
    }

    void SetTimeSteps (size_t timeSteps);
    string ToString () const;
    string ToHtml () const;
    void SetPressureAdjusted (bool adjustPressure)
    {
	m_pressureAdjusted = adjustPressure;
    }
    const DmpObjectInfo& GetDmpObjectInfo () const
    {
	return m_dmpObjectInfo;
    }    
    const vector<ForceNamesOneObject>& GetForcesNames () const
    {
	return m_forceNames;
    }
    bool IsForceAvailable () const
    {
	return m_forceNames.size ();
    }
    bool IsTorqueAvailable () const;

    bool IsTopologicalChangeAvailable () const;
    int GetTopologicalChangeShift () const
    {
	return m_topologicalChangeShift;
    }
    bool GetTopologicalChangeShiftLower () const
    {
	return m_topologicalChangeShift == 1;
    }
    const vector<TopologicalChange>& GetTopologicalChange (
        size_t timeStep, int t1sShift) const;
    size_t GetTopologicalChangeSize () const;
    size_t GetTopologicalChangeTimeSteps () const;

    bool OriginalUsed () const
    {
	return m_useOriginal;
    }
    /**
     * Parse topological changes from the file.
     * in the file: first time step is 1 and topological changes occur 
     * before timeStep
     * in memory: first time step is 0 and topological changes occur 
     * after timeStep
     *
     */
    void ParseTopologicalChanges (
	const string& fileName, size_t ticksForTimeStep, bool t1sShiftLower);
    void ParseTopologicalChanges (const char* arrayName, const char* countName);
    void ParseDMPs (const vector<string>& fileNames,
		    bool useOriginal,
		    const DmpObjectInfo& dmpObjectInfo,
		    const vector<ForceNamesOneObject>& forceNames,
		    bool debugParsing, bool debugScanning);
    DataProperties* GetDataProperties ()
    {
	return &m_dataProperties;
    }
    int GetRotation2D () const
    {
	return m_rotation2D;
    }
    void SetRotation2D (int rotation2D)
    {
	m_rotation2D = rotation2D;
    }
    size_t GetReflectionAxis () const
    {
	return m_reflectAxis;
    }
    void SetReflectionAxis (size_t axis)
    {
	m_reflectAxis = axis;
    }
    float GetBubbleDiameter () const;
    size_t GetRegularGridResolution () const
    {
	return m_regularGridResolution;
    }
    void SetRegularGridResolution (size_t resolution);
    static string GetBaseCacheDir ();
    string GetCacheDir () const;

private:
    void MapPerFoam (FoamParamMethod* foamMethods, size_t n);
    void fixConstraintPoints ();
    void adjustPressureAlignMedians ();
    void adjustPressureSubtractReference ();

    void calculateBodyWraps ();
    void calculateVelocity ();
    void calculateVelocityBody (
	pair< size_t, boost::shared_ptr<BodyAlongTime> > p);
    void calculateStatistics ();
    void storeVelocity (
	const StripIteratorPoint& beforeBegin,
	const StripIteratorPoint& begin,
	const StripIteratorPoint& end,
	const StripIteratorPoint& afterEnd);
    
    template <typename Accumulator>
    void forAllBodiesAccumulateProperty (
	Accumulator* acc, BodyScalar::Enum property);
    template <typename Accumulator, typename GetBodyScalar>
    void forAllBodiesAccumulate (
	Accumulator* acc, GetBodyScalar getBodyScalar);
    void moveInsideOriginalDomain (
        TopologicalChange* tc, const OOBox& originalDomain);

private:
    /**
     * Vector of Foam objects
     */
    Foams m_foams;
    BodiesAlongTime m_bodiesAlongTime;
    DataProperties m_dataProperties;
    /**
     * The AABox for this vector of Foam objects
     */
    G3D::AABox m_boundingBox;
    G3D::AABox m_boundingBoxTorus;
    /**
     * This is the name specified in the ini file.
     */
    string m_name;
    vector<HistogramStatistics> m_histogram;
    bool m_pressureAdjusted;
    vector< vector<TopologicalChange> > m_topologicalChange;
    int m_topologicalChangeShift;
    DmpObjectInfo m_dmpObjectInfo;
    vector<ForceNamesOneObject> m_forceNames;
    bool m_useOriginal;
    int m_rotation2D;
    size_t m_reflectAxis;
    float m_maxDeformationEigenValue;
    size_t m_regularGridResolution;
};

class SimulationGroup
{
public:
    void SetSize (size_t n)
    {
	m_simulation.resize (n);
    }

    size_t size () const
    {
	return m_simulation.size ();
    }

    const Simulation& GetSimulation (size_t i) const
    {
	return m_simulation[i];
    }
    const Simulation& GetSimulation(
	const Settings& settings, ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation(const Settings& settings) const;
    Simulation& GetSimulation (size_t i)
    {
	return m_simulation[i];
    }
    const vector<Simulation>& GetSimulations () const
    {
	return m_simulation;
    }
    string ToString () const;
    float GetBubbleDiameter () const;    
    size_t GetIndex3DSimulation () const
    {
        return getIndexSimulation (3);
    }
    size_t GetIndex2DSimulation () const
    {
        return getIndexSimulation (2);
    }

private:
    size_t getIndexSimulation (size_t spaceDimension) const;

private:
    vector<Simulation> m_simulation;
};


class AverageCache
{
public:
    void SetTopologicalChangeKDE (vtkSmartPointer<vtkImageData> average)
    {
        m_topologicalChangeKDE = average;
    }
    vtkSmartPointer<vtkImageData> GetTopologicalChangeKDE () const
    {
        return m_topologicalChangeKDE;
    }

    void SetVelocity (vtkSmartPointer<vtkImageData> average)
    {
        m_velocityAverage = average;
    }

    vtkSmartPointer<vtkImageData> GetVelocity () const
    {
        return m_velocityAverage;
    }
    
private:
    vtkSmartPointer<vtkImageData> m_topologicalChangeKDE;
    vtkSmartPointer<vtkImageData> m_velocityAverage;
};



/**
 * Pretty print a Simulation
 */
inline ostream& operator<< (ostream& ostr, const Simulation& simulation)
{
    return ostr << simulation.ToString () << endl;
}

inline ostream& operator<< (ostream& ostr, 
			    const SimulationGroup& simulationGroup)
{
    return ostr << simulationGroup.ToString () << endl;
}



#endif //__SIMULATION_H__

// Local Variables:
// mode: c++
// End:
