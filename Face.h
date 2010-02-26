/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "OrientedEdge.h"
#include "Color.h"

class AttributesInfo;
class Body;
class Data;

/**
 * A Face is a oriented list of edges.
 */
class Face : public Element
{
public:
    /**
     * Constructs a Face object
     * @param originalIndex original index for this face
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face(vector<int>& edgeIndexes, vector<Edge*>& edges, 
	 unsigned int originalIndex, Data* data,
	 bool duplicate = false);
    /**
     * Destroys a Face object
     */
    ~Face();
    /**
     * Pretty prints this Face by printing the edges in REVERSE order
     */
    void ReversePrint (ostream& ostr);
    /**
     * Gets the list of oriented edges
     * @return vector of oriented edges
     */
     vector<OrientedEdge*>& GetOrientedEdges ()  
    { return m_edges;}
    OrientedEdge* GetOrientedEdge (int i)  {return m_edges[i];}
    /**
     * Returns the face color
     */
    Color::Name GetColor () ;
    void AddAdjacentBody (Body* body) {m_adjacentBodies.push_back (body);}
     vector<Body*>& GetAdjacentBodies () 
    {
	return m_adjacentBodies;
    }
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, Face& f); 
private:
    /**
     * Index where the color attribute is stored for a face
     */
     static const unsigned int COLOR_INDEX = 0;
    /**
     * Edges that are part of this face
     */
    vector<OrientedEdge*> m_edges;
    vector<Body*> m_adjacentBodies;
    /**
     * Stores information about all vertex attributes
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


#endif //__FACE_H__

// Local Variables:
// mode: c++
// End:
