/**
 * @file FoamAlongTime.h
 * @author Dan R. Lipsa
 *
 * Declaration of FoamAlongTime object
 */
#ifndef __FOAM_ALONG_TIME_H__
#define __FOAM_ALONG_TIME_H__

#include "Comparisons.h"
#include "BodyAlongTime.h"
#include "BodySetStatistics.h"

class Foam;

/**
 * Stores information about a list of DMP files
 */
class FoamAlongTime
{
public:
    typedef vector< boost::shared_ptr<Foam> > Foams;
    typedef vector<BodySetStatistics> FoamsStatistics;
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


    /**
     * Gets the AABox for this vector of Foam objects
     */
    const G3D::AABox& GetAABox () const
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
    float GetBodyProperty (
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

    Foams::iterator GetFoamsBegin ()
    {
	return m_foams.begin ();
    }
    QwtIntervalData GetHistogram (size_t bodyProperty) const
    {
	return GetBodiesAlongTime ().GetHistogram (bodyProperty);
    }

    QwtIntervalData GetHistogram (size_t bodyProperty, size_t timeStep) const
    {
	return m_foamsStatistics[timeStep].GetHistogram (
	    bodyProperty, &GetBodiesAlongTime ());
    }
    float GetMax (BodyProperty::Enum bodyProperty) const
    {
	return GetBodiesAlongTime ().GetMax (bodyProperty);
    }
    float GetMax (BodyProperty::Enum bodyProperty, size_t timeStep) const
    {
	return m_foamsStatistics[timeStep].GetMax (bodyProperty);
    }
    size_t GetMaxCountPerBinIndividual (size_t bodyProperty) const
    {
	return m_maxCountPerBinIndividual[bodyProperty];
    }
    size_t GetMaxCountPerBin (size_t bodyProperty) const
    {
	return GetBodiesAlongTime ().GetMaxCountPerBin (bodyProperty);
    }

    float GetMin (BodyProperty::Enum bodyProperty) const
    {
	return GetBodiesAlongTime ().GetMin (bodyProperty);
    }
    float GetMin (BodyProperty::Enum bodyProperty, size_t timeStep) const
    {
	return m_foamsStatistics[timeStep].GetMin (bodyProperty);
    }
    QwtDoubleInterval GetRange (BodyProperty::Enum bodyProperty) const
    {
	return QwtDoubleInterval (GetMin (bodyProperty), GetMax (bodyProperty));
    }
    QwtDoubleInterval GetRange (
	BodyProperty::Enum bodyProperty, size_t timeStep) const
    {
	return QwtDoubleInterval (
	    GetMin (bodyProperty, timeStep), GetMax (bodyProperty, timeStep));
    }
    size_t GetTimeSteps () const
    {
	return m_foams.size ();
    }
    /**
     * Returns the time steps for which the range of values is in one of the 
     * valueIntervals.
     */
    void GetTimeStepSelection (
	BodyProperty::Enum bodyProperty,
	const vector<QwtDoubleInterval>& valueIntervals,
	vector<bool>* timeStepSelection) const;
    void GetTimeStepSelection (
	BodyProperty::Enum bodyProperty,
	const QwtDoubleInterval& valueInterval,
	vector<bool>* timeStepSelection) const;


    bool ExistsBodyProperty (
	BodyProperty::Enum property,
	size_t bodyId, size_t timeStep) const;

    void PostProcess ();

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
    void calculate (Aggregate aggregate, 
		    FoamLessThanAlong::Corner corner, G3D::Vector3& v);
    void calculateBodyWraps ();
    void calculateHistogram ();
    void calculateHistogram (size_t timeStep);
    void calculateRange ();
    void calculateRange (size_t timeStep);
    void calculateMaxCountPerBin ();

private:
    /**
     * Vector of Foam objects
     */
    Foams m_foams;
    FoamsStatistics m_foamsStatistics;
    /**
     * Each element of the array corresponds to a histogram for a property.
     * It tells us, the maximum number of values we have in a single bin.
     */
    vector<size_t> m_maxCountPerBinIndividual;    
    BodiesAlongTime m_bodiesAlongTime;
    /**
     * The AABox for this vector of Foam objects
     */
    G3D::AABox m_AABox;
    string m_filePattern;
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
