/**
 * @file FoamAlongTime.h
 * @author Dan R. Lipsa
 *
 * Declaration of FoamAlongTime object
 */
#ifndef __FOAM_ALONG_TIME_H__
#define __FOAM_ALONG_TIME_H__

#include "BodyAlongTime.h"
#include "Comparisons.h"
#include "FoamParameters.h"
#include "HistogramStatistics.h"
#include "ConstraintRotation.h"
#include "Force.h"
#include "Utils.h"

class Foam;

/**
 * Stores information about a list of DMP files
 */
class FoamAlongTime
{
public:
    typedef vector< boost::shared_ptr<Foam> > Foams;
    typedef void (Foam::*FoamMethod) ();
    typedef boost::function<void (Foam*)> FoamParamMethod;

public:
    FoamAlongTime ();
    void CacheBodiesAlongTime ();
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Foam objects
     */
    void CalculateBoundingBox ();
    bool Is2D () const;

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
    const Body& GetBody (size_t bodyId, size_t timeStep) const;

    const string& GetFilePattern () const
    {
	return m_filePattern;
    }
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

    double GetMin (BodyProperty::Enum property) const
    {
	return acc::min (GetHistogram (property));
    }

    double GetMax (BodyProperty::Enum property) const
    {
	return acc::max (GetHistogram (property));
    }

    QwtDoubleInterval GetRange (BodyProperty::Enum property) const
    {
	return QwtDoubleInterval (GetMin (property), GetMax (property));
    }


    /**
     * For the histogram associated with each foam this is the maximum of 
     * GetMaxCountPerBin for all foams.
     */
    size_t GetMaxCountPerBinIndividual (BodyProperty::Enum property) const;

    size_t GetTimeSteps () const
    {
	return m_foams.size ();
    }
    /**
     * Returns the time steps for which the range of values is in one of the 
     * valueIntervals.
     */
    void GetTimeStepSelection (
	BodyProperty::Enum property,
	const vector<QwtDoubleInterval>& valueIntervals,
	vector<bool>* timeStepSelection) const;
    void GetTimeStepSelection (
	BodyProperty::Enum property,
	const QwtDoubleInterval& valueInterval,
	vector<bool>* timeStepSelection) const;

    void Preprocess ();

    void SetFilePattern (const string& filePattern)
    {
	m_filePattern = filePattern;
    }
    void SetTimeSteps (size_t timeSteps);
    string ToString () const;
    string ToHtml () const;
    bool IsPressureAdjusted () const
    {
	return m_adjustPressure;
    }
    void SetAdjustPressure (bool adjustPressure)
    {
	m_adjustPressure = adjustPressure;
    }
    const ConstraintRotationNames& GetConstraintRotationNames () const
    {
	return m_constraintRotationNames;
    }    
    const vector<ForceNames>& GetForcesNames () const
    {
	return m_forcesNames;
    }
    bool ForceUsed () const
    {
	return m_forcesNames.size ();
    }

    bool T1sAvailable () const;
    void SetT1sShiftLower (int i)
    {
	m_t1sTimeStepShift = i;
    }
    int GetT1sShiftLower () const
    {
	return m_t1sTimeStepShift;
    }
    const vector<G3D::Vector3>& GetT1s (size_t timeStep) const;
    size_t GetT1sTimeSteps () const;

    bool OriginalUsed () const
    {
	return m_useOriginal;
    }
    /**
     * Parse T1s from the file.
     * in the file: first time step is 1 and T1s occur before timeStep
     * in memory: first time step is 0 and T1s occur after timeStep
     *
     */
    void ParseT1s (
	const string& fileName, size_t ticksForTimeStep, bool shiftT1sLower);
    void ParseDMPs (const vector<string>& fileNames,
		    bool useOriginal,
		    const ConstraintRotationNames& constraintRotationNames,
		    const vector<ForceNames>& forcesNames,
		    bool debugParsing, bool debugScanning);
    FoamParameters* GetFoamParameters ()
    {
	return &m_foamParameters;
    }

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
    void forAllBodiesAccumulate (Accumulator* acc, BodyProperty::Enum property);

private:
    static const vector<G3D::Vector3> NO_T1S;

private:
    /**
     * Vector of Foam objects
     */
    Foams m_foams;
    BodiesAlongTime m_bodiesAlongTime;
    FoamParameters m_foamParameters;
    /**
     * The AABox for this vector of Foam objects
     */
    G3D::AABox m_boundingBox;
    G3D::AABox m_boundingBoxTorus;
    string m_filePattern;
    vector<HistogramStatistics> m_histogram;
    bool m_adjustPressure;
    vector< vector<G3D::Vector3> > m_t1s;
    int m_t1sTimeStepShift;
    ConstraintRotationNames m_constraintRotationNames;
    vector<ForceNames> m_forcesNames;
    bool m_useOriginal;
};


/**
 * Pretty print a FoamAlongTime
 */
inline ostream& operator<< (ostream& ostr, const FoamAlongTime& foamAlongTime)
{
    return ostr << foamAlongTime.ToString () << endl;
}



#endif //__FOAM_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
