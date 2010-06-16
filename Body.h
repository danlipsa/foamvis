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
 * A body is a list of oriented faces
 */
class Body : public Element
{
public:
    typedef vector<boost::shared_ptr<OrientedFace> > OrientedFaces;
    typedef multimap<G3D::Vector3, boost::shared_ptr<OrientedFace> , 
		     VectorLessThanAngle> NormalFaceMap;
public:
    /**
     * Creates a new body
     * @param id the original index for this body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(const vector<int>& faceIndexes,
	 const vector< boost::shared_ptr<Face> >& faces,
	 size_t id, 
	 ElementStatus::Duplicate duplicateStatus = ElementStatus::ORIGINAL);

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
    boost::shared_ptr<OrientedFace>  GetFirstFace ()
    {
	return (*m_normalFaceMap->begin ()).second;
    }

    boost::shared_ptr<OrientedFace>  GetOrientedFace (size_t i) const
    {
	return m_orientedFaces[i];
    }
    boost::shared_ptr<Face>  GetFace (size_t i) const;
    size_t size () const
    {
	return m_orientedFaces.size ();
    }
    /**
     * Caches an edge
     * @param e the edge to cache
     */
    void CacheEdge (boost::shared_ptr<Edge> e)
    {
	m_edges.insert (e);
    }
    /**
     * Cache a vertex
     * @param v the vertex to cache
     */
    void CacheVertex (boost::shared_ptr<Vertex> v)
    {
	m_vertices.insert (v);
    }
    /**
     * Caches edges and vertices
     */
    void CacheEdgesVertices (size_t dimension, bool isQuadratic);
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
    void UpdatePartOf (const boost::shared_ptr<Body>& body);

    NormalFaceMap::iterator GetCurrentNormalFace ()
    {
	return m_currentNormalFace;
    }
    void IncrementNormalFace ()
    {
	++m_currentNormalFace;
    }
    bool HasWrap () const;
    void Unwrap (Foam* foam);

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
	set< boost::shared_ptr<T> >& src,
	vector< boost::shared_ptr<T> >& destTessellation,
	vector< boost::shared_ptr<T> >& destPhysical,
	size_t dimension, bool isQuadratic);

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
    set< boost::shared_ptr<Edge> > m_edges;
    /**
     * Vertices for this body
     */
    set< boost::shared_ptr<Vertex> > m_vertices;
    /**
     * Physical vertices for this body
     */
    vector< boost::shared_ptr<Vertex> > m_physicalVertices;
    /**
     * Tessellation vertices for this body
     */
    vector< boost::shared_ptr<Vertex> > m_tessellationVertices;
    /**
     * Physical edges for this body
     */
    vector< boost::shared_ptr<Edge> > m_physicalEdges;
    /**
     * Tessellation edges for this body
     */
    vector< boost::shared_ptr<Edge> > m_tessellationEdges;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;
};
/**
 * Pretty prints a boost::shared_ptr<Body> 
 * @param ostr where to print
 * @param b what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, const boost::shared_ptr<Body>  b)
{
    return ostr << *b;
}

#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
