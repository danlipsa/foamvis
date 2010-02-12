/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "OrientedFace.h"

class AttributesInfo;
/**
 * A body is a set of faces
 */
class Body : public Element
{
public:
    /**
     * Creates a new body
     * @param originalIndex the original index for this body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(unsigned int originalIndex, 
	 const vector<int>& faceIndexes, vector<Face*>& faces);
    ~Body ();
    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    const vector<OrientedFace*>& GetOrientedFaces() const
    {
	return m_faces;
    }
    /**
     * Does this body have this edge
     * @param e the edge to be tested
     * @return true if the body has the edge, false otherwise
     */
    bool HasEdge (const Edge* e) const
    {
	return m_edges.find (e) != m_edges.end ();
    }
    bool HasVertex (const Vertex* v) const
    {
	return m_vertices.find (v) != m_vertices.end ();
    }
    /**
     * Caches an edge
     * @param e the edge to cache
     */
    void CacheEdge (const Edge* e)
    {
	m_edges.insert (e);
    }
    /**
     * Cache a vertex
     * @param v the vertex to cache
     */
    void CacheVertex (const Vertex* v)
    {
	m_vertices.insert (v);
    }
    /**
     * Caches edges and vertices
     */
    void CacheEdgesVertices ();
    /**
     * Calculates the center
     */
    void CalculateCenter ();
    /**
     * Gets the center
     * @return the center of the body
     */
    const G3D::Vector3& GetCenter () const {return m_center;}
    /**
     * Prety prints a Body
     * @param ostr where to print
     * @param b what to print
     * @return the stream where we printed.
     */
    friend ostream& operator<< (ostream& ostr, Body& b); 
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
private:
    /**
     * Splits a  set of  objects (vertices or  edges) in  physical and
     * tesselation objects.
     * @param src source for the objects
     * @param destTessellation where we store tessellation objects
     * @param destPhysical where we store physical objects
     */
    template <typename T>
    void split (
	set<const T*>& src,
	vector<const T*>& destTessellation,
	vector<const T*>& destPhysical);
    /**
     * Oriented faces that are part of this body.
     */
    vector<OrientedFace*> m_faces;
    /**
     * Edges for this body
     */
    set<const Edge*> m_edges;
    /**
     * Vertices for this body
     */
    set<const Vertex*> m_vertices;
    /**
     * Physical vertices for this body
     */
    vector<const Vertex*> m_physicalVertices;
    /**
     * Tessellation vertices for this body
     */
    vector<const Vertex*> m_tessellationVertices;
    /**
     * Physical edges for this body
     */
    vector<const Edge*> m_physicalEdges;
    /**
     * Tessellation edges for this body
     */
    vector<const Edge*> m_tessellationEdges;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;

    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty prints a Body*
 * @param ostr where to print
 * @param b what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, Body* b)
{
    return ostr << *b;
}

#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
