/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "ColoredElement.h"

class AttributesInfo;
class Body;
class Data;
class Edge;
class OrientedEdge;

/**
 * A Face is a oriented list of edges.
 */
class Face : public ColoredElement
{
public:
    typedef vector<OrientedEdge*> OrientedEdges;

public:
    /**
     * Constructs a Face object
     * @param originalIndex original index for this face
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face(vector<int>& edgeIndexes, vector<Edge*>& edges, 
	 size_t originalIndex, Data* data,
	 bool duplicate = false);
    Face (const Face& original);
    Face (Edge* edge, size_t originalIndex);

    /**
     * Destroys a Face object
     */
    ~Face();
    /**
     * Gets the list of oriented edges
     * @return vector of oriented edges
     */
    const vector<OrientedEdge*>& GetOrientedEdges () const
    {
	return m_edges;
    }
    size_t GetEdgeCount () const
    {
	return m_edges.size ();
    }
    vector<OrientedEdge*>& GetOrientedEdges ()
    {
	return m_edges;
    }
    OrientedEdge* GetOrientedEdge (int i) const
    {
	return m_edges[i];
    }
    void AddAdjacentBody (Body* body) 
    {
	m_adjacentBodies.push_back (body);
    }
    vector<Body*>& GetAdjacentBodies ()
    {
	return m_adjacentBodies;
    }

    /**
     * For all the  edges in the face, add the  face as being adjacent
     * to the edge
     */
    void UpdateEdgesAdjacency ();
    void ClearEdgesAdjacency ();
    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;
    bool operator== (const Face& face) const;
    G3D::Vector3 GetNormal () const;
    Face* CreateDuplicate (const G3D::Vector3& newBegin) const;
    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Face::m_infos);
    }
    bool IsClosed () const;


public:
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend ostream& operator<< (ostream& ostr, const Face& f); 

private:
    /**
     * Edges that are part of this face
     */
    OrientedEdges m_edges;
    vector<Body*> m_adjacentBodies;
private:
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
inline ostream& operator<< (ostream& ostr, const Face* f)
{
    return ostr << *f;
}


#endif //__FACE_H__

// Local Variables:
// mode: c++
// End:
