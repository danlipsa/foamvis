/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "Comparisons.h"
#include "Element.h"

class AttributesInfo;
class Edge;
class Face;
class OrientedFace;
class OrientedEdge;
class Vertex;

/**
 * A body is a set of faces
 */
class Body : public Element
{
public:
    typedef vector<OrientedFace*> OrientedFaces;
    typedef multimap<G3D::Vector3, OrientedFace*, 
		     VectorLessThanAngle> NormalFaceMap;
public:
    /**
     * Creates a new body
     * @param id the original index for this body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(vector<int>& faceIndexes, vector<Face*>& faces,
	 size_t id, ElementStatus::Name status = ElementStatus::ORIGINAL);
    ~Body ();

    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    OrientedFaces& GetOrientedFaces ()
    {
	return m_orientedFaces;
    }
    const OrientedFaces& GetOrientedFaces () const
    {
	return m_orientedFaces;
    }

    NormalFaceMap::const_iterator FindNormalFace (
	const G3D::Vector3& normal) const;
    NormalFaceMap& GetNormalFaceMap ()
    {
	return *m_normalFaceMap.get ();
    }
    OrientedFace* GetFirstFace ()
    {
	return (*m_normalFaceMap->begin ()).second;
    }

    OrientedFace* GetOrientedFace (size_t i) const
    {
	return m_orientedFaces[i];
    }
    Face* GetFace (size_t i) const;
    size_t size () const
    {
	return m_orientedFaces.size ();
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
    void PrintDomains (ostream& ostr) const;
    void UpdatePartOf ();
    void ClearPartOf ();

    NormalFaceMap::iterator GetCurrentNormalFace ()
    {
	return m_currentNormalFace;
    }
    void IncrementNormalFace ()
    {
	++m_currentNormalFace;
    }
    bool HasWrap () const;
    ostream& PrintFaceEdgeInformation (ostream& ostr) const;

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

    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Body::m_infos);
    }

private:
    /**
     * Oriented faces that are part of this body.
     */
    OrientedFaces m_orientedFaces;
    boost::scoped_ptr<NormalFaceMap> m_normalFaceMap;
    /**
     * Points to the next group of normals
     */
    NormalFaceMap::iterator m_currentNormalFace;
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
