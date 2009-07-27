/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include <vector>
#include <iostream>
#include "OrientedEdge.h"
using namespace std;

/**
 * A Face is a oriented list of edges.
 */
class Face
{
public:
    /**
     * Constructs a Face object
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face(const vector<int>& edgeIndexes, vector<Edge*>& edges);
    /**
     * Destroys a Face object
     */
    ~Face();
    /**
     * Pretty prints this Face by printing the edges in REVERSE order
     */
    void ReversePrint (ostream& ostr);
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, Face& f); 
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
