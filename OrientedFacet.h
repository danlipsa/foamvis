/**
 * @file OrientedFacet.h
 * @author Dan R. Lipsa
 * 
 * Declaration for the OrientedFacet class
 */
#ifndef __ORIENTED_FACET_H__
#define __ORIENTED_FACET_H__

#include <iostream>
#include "Facet.h"
using namespace std;

/**
 * An oriented facet is a facet  (list of edges) that can have its edges
 * read in direct or reversed order.
 */
class OrientedFacet
{
public:
    /**
     * Constructs a OrientedFacet object
     * @param facet a pointer to a Facet
     * @param reversed if true the edges in the Facet should be read in
     *        reverse order,  otherwise they should be  read in direct
     *        order
     */
    OrientedFacet(Facet* facet, bool reversed)
    {m_facet = facet; m_reversed = reversed;}
    /**
     * Pretty print for the OrientedFacet object
     */
    friend ostream& operator<< (ostream& ostr, OrientedFacet& of);


private:
    /**
     * Object that has information about the edges in this OrientedFacet
     */
    Facet* m_facet;
    /**
     * If true,  edges in the refering  Facet object should  be read in
     * reversed order
     */
    bool m_reversed;
};


#endif

// Local Variables:
// mode: c++
// End:
