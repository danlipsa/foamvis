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
class AttributesInfo;

/**
 * A Face is a oriented list of edges.
 */
class Face : public Element
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
    const vector<OrientedEdge*> GetOrientedEdges () const 
    { return m_edges;}
    Qt::GlobalColor GetColor ();
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, Face& f); 

    static void SetDefaultAttributes (AttributesInfo& info);
private:
    const unsigned int COLOR_INDEX;
    /**
     * Edges that are part of this face
     */
    vector<OrientedEdge*> m_edges;
    static AttributesInfo* m_infos;
};


#endif

// Local Variables:
// mode: c++
// End:
