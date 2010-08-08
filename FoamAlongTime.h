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
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Foam objects
     */
    void CalculateAABox ();
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


    size_t GetTimeSteps () const
    {
	return m_foams.size ();
    }

    void SetTimeSteps (size_t timeSteps)
    {
	m_foams.resize (timeSteps);
    }

    Foams::iterator GetFoamsBegin ()
    {
	return m_foams.begin ();
    }

    /**
     * Gets the AABox for this vector of Foam objects
     */
    const G3D::AABox& GetAABox () const
    {
	return m_AABox;
    }
    void CacheBodiesAlongTime ();
    void PostProcess ();
    BodiesAlongTime& GetBodiesAlongTime ()
    {
	return m_bodiesAlongTime;
    }
    const BodiesAlongTime& GetBodiesAlongTime () const
    {
	return m_bodiesAlongTime;
    }
    void SetFilePattern (const string& filePattern)
    {
	m_filePattern = filePattern;
    }
    const string& GetFilePattern () const
    {
	return m_filePattern;
    }
    float GetMin (BodyProperty::Enum i) const
    {
	return GetBodiesAlongTime ().GetMin (i);
    }
    float GetMax (BodyProperty::Enum i) const
    {
	return GetBodiesAlongTime ().GetMax (i);
    }
    float GetBodyProperty (
	BodyProperty::Enum property,
	size_t bodyId, size_t timeStep) const;
    const Body& GetBody (size_t bodyId, size_t timeStep) const;
    bool ExistsBodyProperty (
	BodyProperty::Enum property,
	size_t bodyId, size_t timeStep) const;
    QwtIntervalData GetHistogram (size_t bodyProperty) const
    {
	return GetBodiesAlongTime ().GetHistogram (bodyProperty);
    }

public:
    /**
     * Pretty print the FoamFile object
     */
    friend ostream& operator<< (
	ostream& ostr, const FoamAlongTime& dataAlongTime);

private:
    void calculateHistogram ();
    void calculateHistogram (size_t timeStep);
    void calculateRange ();
    void calculateRange (size_t timeStep);

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
private:
    /**
     * Vector of Foam objects
     */
    Foams m_foams;
    FoamsStatistics m_foamsStatistics;
    BodiesAlongTime m_bodiesAlongTime;
    /**
     * The AABox for this vector of Foam objects
     */
    G3D::AABox m_AABox;
    string m_filePattern;
};

#endif //__FOAM_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
