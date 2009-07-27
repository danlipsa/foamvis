/**
 * @file   Facet.h
 * @author Dan R. Lipsa
 *
 * Declaration of Facet class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include <vector>
#include <iostream>
#include "OrientedEdge.h"
using namespace std;

/**
 * A Facet is a oriented list of edges.
 */
class Facet
{
public:
    /**
     * Constructs a Facet object
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Facet(const vector<int>& edgeIndexes, vector<Edge*>& edges);
    /**
     * Destroys a Facet object
     */
    ~Facet();
    /**
     * Pretty prints this Facet by printing the edges in REVERSE order
     */
    void ReversePrint (ostream& ostr);
    /**
     * Pretty prints this Facet by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, Facet& f); 
private:
    /**
     * Edges that are part of this face
     */
    vector<OrientedEdge*> m_edges;
};


#endif

// Local Variables:
// mode: c++
// End:
