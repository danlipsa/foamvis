/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include <vector>
#include "OrientedFacet.h"
using namespace std;

/**
 * A body is a set of facets
 */
class Body
{
public:
    /**
     * Creates a new body
     * @param faceIndexes 0 based indexes into a vector of Facet objects
     * @param facets vector of Facet objects
     */
    Body(const vector<int>& faceIndexes, vector<Facet*>& facets);
    /**
     * Deletes the memory allocated to store a Body object
     */
    ~Body();
    /**
     * Prety print a body
     */
    friend ostream& operator<< (ostream& ostr, Body& b); 

private:
    /**
     * Facets that are part of this body.
     */
    vector<OrientedFacet*> m_facets;
};

#endif

// Local Variables:
// mode: c++
// End:
