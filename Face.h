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
#include "lexYacc.h"
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
    const vector<OrientedEdge*>& GetOrientedEdges () const 
    { return m_edges;}
	Color::Name GetColor () const;
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, Face& f); 
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
private:
    const static unsigned int COLOR_INDEX = 0;
    /**
     * Edges that are part of this face
     */
    vector<OrientedEdge*> m_edges;
    /**
     * Information about Face attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty prints a Face*
 * @param ostr where to print the Face object
 * @param f what to print
 * @return stream where to print other data
 */
inline ostream& operator<< (ostream& ostr, Face* f)
{
    return ostr << *f;
}


#endif

// Local Variables:
// mode: c++
// End:
