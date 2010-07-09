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
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Foam objects
     */
    void CalculateAABox ();
    /**
     * Gets the vector of Foam objects
     * @return vector of data objects
     */
    boost::shared_ptr<Foam> GetFoam (size_t time)
    {
	return m_foams[time];
    }

    boost::shared_ptr<const Foam> GetFoam (size_t time) const
    {
	return m_foams[time];
    }


    size_t GetFoamsSize () const
    {
	return m_foams.size ();
    }

    void SetFoamsSize (size_t size)
    {
	m_foams.resize (size);
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


public:
    /**
     * Pretty print the FoamFile object
     */
    friend ostream& operator<< (
	ostream& ostr, const FoamAlongTime& dataAlongTime);

private:
    /**
     * Calculates the low/high point for the AABox of this list of Foam objects
     * @param aggregate functor to be applied to the list of data objects
     * @param corner pointer to a function member of Foam that returns
     * the low or high corner of AABox of the data object
     * @param v where to store the low/high point
     */
    void Calculate (Aggregate aggregate, 
		    FoamLessThanAlong::Corner corner, G3D::Vector3& v);
    void CalculateBodyWraps ();
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
};

#endif //__FOAM_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
