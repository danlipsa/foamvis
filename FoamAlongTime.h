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
#include "Statistics.h"

class Foam;

/**
 * Stores information about a list of DMP files
 */
class FoamAlongTime
{
public:
    typedef vector< boost::shared_ptr<Foam> > Foams;

    /**
     * Functor applied to a collection of Foam objects
     */
    typedef Foams::iterator (*Aggregate)(Foams::iterator first,
					 Foams::iterator last,
					 FoamLessThanAlong lessThanAlong);
public:
    FoamAlongTime ();
    void CacheBodiesAlongTime ();
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Foam objects
     */
    void CalculateAABox ();
    size_t GetDimension () const;


    /**
     * Gets the AABox for this vector of Foam objects
     */
    const G3D::AABox& GetBoundingBox () const
    {
	return m_AABox;
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
    double GetBodyPropertyValue (
	BodyProperty::Enum property,
	size_t bodyId, size_t timeStep) const;

    const string& GetFilePattern () const
    {
	return m_filePattern;
    }
    /**
     * Gets the vector of Foam objects
     * @return vector of data objects
     */
    boost::shared_ptr<Foam> GetFoam (size_t timeStep)
    {
	return m_foams[timeStep];
    }

    boost::shared_ptr<const Foam> GetFoam (size_t timeStep) const
    {
	return m_foams[timeStep];
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


    bool ExistsBodyProperty (
	BodyProperty::Enum property,
	size_t bodyId, size_t timeStep) const;

    void Preprocess ();

    void SetFilePattern (const string& filePattern)
    {
	m_filePattern = filePattern;
    }
    void SetTimeSteps (size_t timeSteps);
    string ToString () const;

private:
    /**
     * Calculates the low/high point for the AABox of this list of Foam objects
     * @param aggregate functor to be applied to the list of data objects
     * @param corner pointer to a function member of Foam that returns
     * the low or high corner of AABox of the data object
     * @param v where to store the low/high point
     */
    void calculateAggregate (Aggregate aggregate, 
		    FoamLessThanAlong::Corner corner, G3D::Vector3& v);
    void calculateBodyWraps ();
    void calculateVelocity ();
    void calculateStatistics ();
    void storeVelocity (
	const StripIterator::Point& beforeBegin,
	const StripIterator::Point& begin,
	const StripIterator::Point& end,
	const StripIterator::Point& afterEnd);
    
    template <typename Accumulator>
    void forAllBodiesAccumulate (Accumulator* acc, BodyProperty::Enum property);

private:
    /**
     * Vector of Foam objects
     */
    Foams m_foams;
    BodiesAlongTime m_bodiesAlongTime;
    /**
     * The AABox for this vector of Foam objects
     */
    G3D::AABox m_AABox;
    string m_filePattern;
    vector<HistogramStatistics> m_histogram;
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
