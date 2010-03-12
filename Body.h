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
    typedef vector<OrientedFace*> OrientedFaces;
    class FaceIntersectionMargin
    {
    public:
	enum Margin
	{
	    BEFORE_FIRST_AFTER_SECOND,
	    AFTER_FIRST_BEFORE_SECOND
	};

	struct FaceEdgeIndex
	{
	    FaceEdgeIndex () : m_face (0), m_edgeIndex (0) {}
	    FaceEdgeIndex (const OrientedFace* face, size_t edgeIndex) :
		m_face (face), m_edgeIndex (edgeIndex) {}
	    const OrientedFace* m_face;
	    /**
	     * Index in the oriented face
	     */
	    size_t m_edgeIndex; 
	};

    public:
	FaceIntersectionMargin (
	    Margin margin,
	    const FaceEdgeIndex& first,
	    const FaceEdgeIndex& second) :
	    m_margin (margin), m_first(first), m_second(second) {}
	FaceIntersectionMargin () :
	    m_margin (BEFORE_FIRST_AFTER_SECOND) {}
	void SetMargin (Margin margin)
	{
	    m_margin = margin;
	}
	Margin GetMargin () const
	{
	    return m_margin;
	}
	const FaceEdgeIndex& GetFirst () const
	{
	    return m_first;
	}
	const FaceEdgeIndex& GetSecond () const
	{
	    return m_second;
	}
	const OrientedFace& GetFirstFace () const
	{
	    return *m_first.m_face;
	}
	const OrientedFace& GetSecondFace () const
	{
	    return *m_second.m_face;
	}
	ostream& operator<< (ostream& ostr) const;

    public:
	static void GetTriangle (
	    const FaceEdgeIndex& first, const FaceEdgeIndex& second,
	    boost::array<G3D::Vector3, 3>* triangle);
	friend ostream& operator<< (ostream& ostr,
				    const FaceEdgeIndex& fei);
	friend ostream& operator<< (
	    ostream& ostr, const Body::FaceIntersectionMargin& fim);

    private:
	Margin m_margin;
	FaceEdgeIndex m_first;
	FaceEdgeIndex m_second;
    };

public:
    /**
     * Creates a new body
     * @param originalIndex the original index for this body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(vector<int>& faceIndexes, vector<Face*>& faces,
	 size_t originalIndex, Data* data,
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
    OrientedFace* GetOrientedFace (size_t i) const
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
    void* ProcessTorusInit ()
    {
	ProcessForTorus* p = new ProcessForTorus (this);
	p->Initialize ();
	return p;
    }

    bool ProcessTorusStep (void* p)
    {
	return static_cast<ProcessForTorus*>(p)->Step ();
    }
    
    void ProcessTorusEnd (void* p)
    {
	ProcessForTorus* process = static_cast<ProcessForTorus*>(p);
	process->End ();
	delete process;
    }

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
    class ProcessForTorus
    {
    public:
	ProcessForTorus (Body* body) : m_body (body) {}
	void Initialize ();
	bool Step ();
	void End ();
    private:
	list<FaceIntersectionMargin> m_queue;
	Body* m_body;
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
    OrientedFace* fitAndDuplicateFace (
	const FaceIntersectionMargin& faceIntersection);
    bool fitFace (const OrientedFace& face, 
		  const boost::array<G3D::Vector3, 3>& triangle,
		  G3D::Vector3* translation);
    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Body::m_infos);
    }
    
	
private:
    static void GetFaceIntersectionMargins (
	const OrientedFace& firstFace, const OrientedFace& secondFace,
	FaceIntersectionMargin* one, FaceIntersectionMargin* two);


private:
    /**
     * Oriented faces that are part of this body.
     */
    OrientedFaces m_faces;
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
