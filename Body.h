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
class Data;
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
    Body(vector<int>& faceIndexes, vector<Face*>& faces,
	 unsigned int originalIndex, Data* data,
	 bool duplicate = false);
    ~Body ();
    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    vector<OrientedFace*>& GetOrientedFaces()
    {
	return m_faces;
    }
    OrientedFace* GetOrientedFace (unsigned int i) const
    {
	return m_faces[i];
    }
    /**
     * Does this body have this edge
     * @param e the edge to be tested
     * @return true if the body has the edge, false otherwise
     */
    bool HasEdge (Edge* e) const
    {
	return m_edges.find (e) != m_edges.end ();
    }
    bool HasVertex (Vertex* v) const
    {
	return m_vertices.find (v) != m_vertices.end ();
    }
    /**
     * Caches an edge
     * @param e the edge to cache
     */
    void CacheEdge (Edge* e)
    {
	m_edges.insert (e);
    }
    /**
     * Cache a vertex
     * @param v the vertex to cache
     */
    void CacheVertex (Vertex* v)
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
    const G3D::Vector3& GetCenter () const
    {
	return m_center;
    }
    void PrintDomains (ostream& ostr) const
    {
	Vertex::PrintDomains (ostr, m_vertices);
    }
    void UpdateFacesAdjacency ();

public:
    /**
     * Prety prints a Body
     * @param ostr where to print
     * @param b what to print
     * @return the stream where we printed.
     */
    friend ostream& operator<< (ostream& ostr, const Body& b);
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);

private:
    struct Triangle
    {
	enum
	{
	    BEFORE_AFTER,
	    AFTER_BEFORE
	} m_edges;
	OrientedFace* m_first;
	/**
	 * Index in the first face (not oriented face)
	 */
	size_t m_intersectionEdgeFirst;
	OrientedFace* m_second;
	/**
	 * Index in the second face (not oriented face)
	 */
	size_t m_intersectionEdgeSecond;
    };

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
	set<T*>& src, vector<T*>& destTessellation, vector<T*>& destPhysical);
    OrientedFace* fitFace (const Triangle& triangle);
	
private:
    static void getTrianglesFromFaceIntersection (
	OrientedFace& firstFace, OrientedFace& secondFace,
	Triangle* firstTriangle, Triangle* secondTriangle);


private:
    /**
     * Oriented faces that are part of this body.
     */
    vector<OrientedFace*> m_faces;
    /**
     * Edges for this body
     */
    set<Edge*> m_edges;
    /**
     * Vertices for this body
     */
    set<Vertex*> m_vertices;
    /**
     * Physical vertices for this body
     */
    vector<Vertex*> m_physicalVertices;
    /**
     * Tessellation vertices for this body
     */
    vector<Vertex*> m_tessellationVertices;
    /**
     * Physical edges for this body
     */
    vector<Edge*> m_physicalEdges;
    /**
     * Tessellation edges for this body
     */
    vector<Edge*> m_tessellationEdges;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;

private:
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
inline ostream& operator<< (ostream& ostr, const Body* b)
{
    return ostr << *b;
}

#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
