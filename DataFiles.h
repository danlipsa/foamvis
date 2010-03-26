/**
 * @file DataFiles.h
 * @author Dan R. Lipsa
 *
 * A list of Data objects
 */
#ifndef __DATA_FILES_H__
#define __DATA_FILES_H__

#include "Comparisons.h"
class Data;

/**
 * Stores information about a list of DMP files
 */
class DataFiles
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
    /**
     * Pretty print the DataFile object
     */
    friend ostream& operator<< (ostream& ostr, DataFiles& dataFiles);

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
    /**
     * Vector of Data objects
     */
    vector<Data*> m_data;
    /**
     * The AABox for this vector of Data objects
     */
    G3D::AABox m_AABox;
};

#endif //__DATA_FILES_H__

// Local Variables:
// mode: c++
// End:
