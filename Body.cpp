/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include<functional>
#include <algorithm>
#include "Body.h"
#include "Element.h"

/**
 * STL unary  function that converts a  signed index into  a vector of
 * Facet  objects into a  OrientedFacet object.  A negative  index means
 * that the Facet object is listed  in reverse order in the Body object
 * than in the vector of Facet objects.
 */
class indexToOrientedFacet : public unary_function<int, OrientedFacet*>
{
public:
    /**
     * Constructor
     * @param facets vector of Facet pointers. This is where the indexes point to.
     */
    indexToOrientedFacet(vector<Facet*>& facets): m_facets(facets) {}
    /**
     * Converts a signed integer into a OrientedFacet
     * @param i index into a vector of Facet pointers
     * @return an OrientedFacet pointer
     */
    OrientedFacet* operator() (int i)
    {
	bool reversed = false;
	if (i < 0)
	{
	    i = -i;
	    reversed = true;
	}
	return new OrientedFacet(m_facets[i], reversed);
    }
private:
    /**
     * Vector of Facet pointers
     */
    vector<Facet*>& m_facets;
};



Body::~Body()
{
    for_each(m_facets.begin(), m_facets.end(), DeleteElementPtr<OrientedFacet>);
}

Body::Body(const vector<int>& faceIndexes, vector<Facet*>& facets)
{
    m_facets.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_facets.begin(), 
	       indexToOrientedFacet(facets));
}

ostream& operator<< (ostream& ostr, Body& b)
{
    if (&b == 0)
	ostr << "NULL";
    else
	PrintElementPtrs<OrientedFacet> (ostr, b.m_facets, 
					"facets part of the body", true);
    return ostr;
}
