/**
 * @file Simulation.h
 * @author Dan R. Lipsa
 * @brief A time-dependent foam simulation
 * @ingroup data model
 *
 * @defgroup data Data
 * Creates, processes and stores foam simulation data.
 *
 * @section sec_torus Unwrapping of edges, faces and bodies for the Torus model
 * @subsection sec_onedge Executed when creating an edge (may create duplicate vertices)
 *
 <pre>
The begin vertex (and the middle vertex in quadratic mode) of an edge is 
always defined in the data file (it's not a duplicate).
  if (edge is marked with a *)
    the end vertex is defined in the data file (no DUPLICATE needed)
  else if (edge is marked with a + or -)
  {
    create a DUPLICATE of the end vertex by translating it
      one domain up for a + and one domain down for a - along each of 
      the three axes. The translation is done relative to the domain where the 
      ORIGINAL end vertex is defined in the data file. This means that
      (* * *) is the domain where the begin vertex is defined in the data file
      NOT the original domain.
  }
 </pre>
 * 
 * @subsection sec_onface Executed when creating a face (may create duplicate edges)
 *
<pre>
  first vertex of the face is defined in the data file (not a DUPLICATE).
  beginVertex = first vertex of the face
  foreach (currentEdge, edges in the face)
  {
    if (the beginVertex does not match the begin vertex of the currentEdge) {
      create a DUPLICATE of currentEdge starting at beginVertex
      set currentEdge to point to the DUPLICATE
    }
    set beginVertex  to be the end vertex of currentEdge
  }
</pre>
 * 
 * @subsection sec_onbody Executed when creating a body (may create duplicate faces)
 *
<pre>

add the first face to a queue
while (no more items in the queue)
{
   remove a face from the queue, translate it if needed and mark it visited.
   Add all adjacent faces of the face to the queue.
   if several faces fit, choose the face with smallest angle between its 
   normal and the original face normal.
}

</pre>
 *
 *
 * @section sec_physical_tesselation Physical and tesselation edges and vertices
 * In 2D we don't have physical edges. A vertex is "physical" if has
 * >= 3 edges adjacent to it.
 *
 * In 3D, an edge is physical if it has 6 AdjacentOrientedFace is part of. (is
 * adjacent with 3 faces)
 * An vertex is physical if it has 4 physical edges adjacent to it.
 *
 * @section sec_t1s T1s storage format
 * T1s can be stored either in an external file or in varibles inside
 * the file for a simulation time step.
 * @subsection sec_t1s_file Format of additional text file containing T1s
 * A line that starts with a # is a comment line \n
 * Each line contains three entries separated by space: time_step, x, y
 * where x and y are the coordinates of the T1 in object space \n
 * The first time step is 1. \n
 * A T1 labeled with timestep T occurs between T and T+1.\n

 * @subsection sec_t1s_variables Format of variables inside .dmp containing T1s
 * T1s are stored in two variables \c t1positions and \c num_pops_step. 
 * \c num_pops_step stores the number of T1s that happened before this time 
 * step. \c t1positions[0..3][i] stores the x, y, z, and the type of %T1 
 * for topological change \c i.
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
#include "T1.h"

class Foam;
class OOBox;
class Settings;

/**
 * @brief Data for all time-steps in a foam simulation.
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
    Dimension::Enum GetDimension () const;
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
    /**
     * this BB is potentially larger than GetBoundingBox () if it is specified
     * in the ini file. (it should be, otherwise saving individual time steps is
     * not done correctly)
     */
    void SetBoundingBoxAllTimeSteps (const G3D::AABox& box)
    {
        m_boundingBoxAllTimeSteps = box;
    }
    G3D::AABox GetBoundingBoxAllTimeSteps () const;
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


    const HistogramStatistics& GetHistogramScalar (
        BodyScalar::Enum property) const
    {
	return m_histogramScalar[property];
    }

    double GetMinScalar (BodyScalar::Enum property) const
    {
	return acc::min (GetHistogramScalar (property));
    }

    double GetMaxScalar (BodyScalar::Enum property) const
    {
	return acc::max (GetHistogramScalar (property));
    }
    float GetMaxDeformationEigenValue () const
    {
	return m_maxDeformationEigenValue;
    }
    QwtDoubleInterval GetIntervalScalar (BodyScalar::Enum property) const
    {
	return QwtDoubleInterval (
            GetMinScalar (property), GetMaxScalar (property));
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
    string GetInfo () const;
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
    bool OriginalUsed () const
    {
	return m_useOriginal;
    }
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
    boost::array<int, 6> GetExtentResolution () const;
    float GetOneVoxelInObjectSpace () const;

    /**
     * @{
     * @name T1
     */
    bool IsT1Available () const;
    bool GetT1ShiftLower () const
    {
	return m_t1Shift == 1;
    }
    const vector<T1>& GetT1 (size_t timeStep, int t1sShift) const;
    vtkSmartPointer<vtkPolyData> GetT1Vtk (size_t timeStep, int t1sShift) const;
    const char* GetT1VtkName () const
    {
        return "T1";
    }
    string GetT1Info (size_t timeStep, int t1sShift) const;
    vtkSmartPointer<vtkImageData> GetT1KDE (
        size_t timeStep, size_t subStep, int t1Shift,
        float sigmaInBubbleDiameters) const;

    size_t GetT1CountAllTimeSteps () const;
    size_t GetMaxT1CountPerTimeStep () const;
    size_t GetT1TimeSteps () const;
    /**
     * @brief Parse topological changes from a file.
     *
     * in the file: first time step is 1 and topological changes occur 
     * before timeStep
     * in memory: first time step is 0 and topological changes occur 
     * after timeStep
     *
     */
    void ParseT1s (
	const string& fileName, size_t ticksForTimeStep, bool t1sShiftLower);
    void ParseT1s (const char* arrayName, const char* countName);
    // @}

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
    void calculateT1TypeCount ();
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
        T1* tc, const OOBox& originalDomain);


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
    G3D::AABox m_boundingBoxAllTimeSteps;
    G3D::AABox m_boundingBoxTorus;
    /**
     * This is the name specified in the ini file.
     */
    string m_name;
    vector<HistogramStatistics> m_histogramScalar;
    double m_meanVolume;
    bool m_pressureAdjusted;
    vector< vector<T1> > m_t1;
    int m_t1Shift;
    DmpObjectInfo m_dmpObjectInfo;
    vector<ForceNamesOneObject> m_forceNames;
    bool m_useOriginal;
    int m_rotation2D;
    size_t m_reflectAxis;
    float m_maxDeformationEigenValue;
    size_t m_regularGridResolution;
    boost::array<size_t, T1Type::COUNT> m_t1TypeCount;
};

/**
 * @brief A list of foam simulations
 */
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
