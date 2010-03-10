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
    struct Hash
    {
	size_t operator() (const Face& face) const
	{
	    std::size_t seed = 0;
	    boost::hash_combine (seed, face.GetOriginalIndex ());
	    boost::hash_combine (seed, *face.GetOrientedEdge (0)->GetBegin ());
	    return seed;
	}
	size_t operator () (const Face* f) const
	{
	    return operator() (*f);
	}
    };
    typedef vector<OrientedEdge*> OrientedEdges;

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
    void ReversePrint (ostream& ostr) const;
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
    /**
     * Returns the face color
     */
    Color::Name GetColor () const;
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
     * Index where the color attribute is stored for a face
     */
    const static unsigned int COLOR_INDEX = 0;
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
