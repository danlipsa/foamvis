/**
 * @file DataAlongTime.h
 * @author Dan R. Lipsa
 *
 * Declaration of DataAlongTime object
 */
#ifndef __DATA_ALONG_TIME_H__
#define __DATA_ALONG_TIME_H__

#include "Comparisons.h"
#include "BodiesAlongTime.h"

class Data;

/**
 * Stores information about a list of DMP files
 */
class DataAlongTime
{
public:
    /**
     * Iterator over a vector of Data
     */
    typedef vector<Data*>::iterator IteratorData;
    /**
     * Functor applied to a collection of Data objects
     */
    typedef IteratorData (*Aggregate)(IteratorData first,
				      IteratorData last,
				      DataLessThanAlong lessThanAlong);
public:
    /**
     * Calculate the  axially aligned bounding box for  this vector of
     * Data objects
     */
    void CalculateAABox ();
    /**
     * Gets the vector of Data objects
     * @return vector of data objects
     */
    vector<Data*>& GetData () {return m_data;}
    /**
     * Gets the AABox for this vector of Data objects
     */
     G3D::AABox& GetAABox () {return m_AABox;}
    void CacheBodiesAlongTime ();
    void PostProcess ();
    BodiesAlongTime& GetBodiesAlongTime ()
    {
	return m_bodiesAlongTime;
    }

public:
    /**
     * Pretty print the DataFile object
     */
    friend ostream& operator<< (
	ostream& ostr, const DataAlongTime& dataAlongTime);

private:
    /**
     * Calculates the low/high point for the AABox of this list of Data objects
     * @param aggregate functor to be applied to the list of data objects
     * @param corner pointer to a function member of Data that returns
     * the low or high corner of AABox of the data object
     * @param v where to store the low/high point
     */
    void Calculate (Aggregate aggregate, 
		    DataLessThanAlong::Corner corner, G3D::Vector3& v);
    void CalculateBodyCenterWraps ();
private:
    /**
     * Vector of Data objects
     */
    vector<Data*> m_data;
    BodiesAlongTime m_bodiesAlongTime;
    /**
     * The AABox for this vector of Data objects
     */
    G3D::AABox m_AABox;
};

#endif //__DATA_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
