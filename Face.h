/**
 * @file   Face.h
 * @author Dan R. Lipsa
 * @brief A bubble represented as a list of faces, a face is a
 *        oriented list of edges.
 * @ingroup data model
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "Element.h"
#include "Comparisons.h"
#include "AdjacentBody.h"

class AttributesInfo;
class Body;
class ConstraintEdge;
class Foam;
class Edge;
class OrientedEdge;
class OOBox;

/**
 * @brief A bubble represented as a list of faces, a face is a
 *        oriented list of edges.
 */
class Face : public Element
{
public:
    typedef vector< boost::shared_ptr<OrientedEdge> > OrientedEdges;

public:
    /**
     * Constructs a Face object
     */
    Face (const vector<int>& edgeIndexes,
	  const vector< boost::shared_ptr<Edge> >& edges, 
	  size_t id,
	  ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    Face (const vector< boost::shared_ptr<Edge> >& edges, size_t id);
    Face (const Face& original);
    Face (const boost::shared_ptr<Edge>& firstEdge, size_t id);


    void AddAdjacentBody (boost::shared_ptr<Body>  body, 
			  size_t orientedFaceIndex)
    {
	m_adjacentBodies.push_back (AdjacentBody (body, orientedFaceIndex));
    }
    void AddEdge (boost::shared_ptr<Edge> edge);
    void SetNormal ();
    const G3D::Vector3& GetCenter () const
    {
	return m_center;
    }
    size_t GetEdgeCount () const
    {
	return m_orientedEdges.size ();
    }

    const AdjacentBody& GetAdjacentBody (bool faceReversed = false) const;
    size_t GetAdjacentBodySize () const
    {
	return m_adjacentBodies.size ();
    }
    void GetVertexSet (VertexSet* vertexSet) const;
    void GetEdgeSet (EdgeSet* edgeSet) const;
    EdgeSet GetEdgeSet () const
    {
	EdgeSet set;
	GetEdgeSet (&set);
	return set;
    }
    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;

    bool operator== (const Face& face) const;
    bool fuzzyEq (const Face& other) const;
    bool operator< (const Face& other) const;

    G3D::Vector3 GetNormal () const
    {
	return m_normal;
    }
    G3D::Plane GetPlane () const;

    float GetPerimeter () const
    {
	return m_perimeter;
    }
    /**
     * @pre {CalculateCentroidAndArea executed.}
     */
    float GetArea () const
    {
	return m_area;
    }
    boost::shared_ptr<Face> GetDuplicate (
	const OOBox& periods, const G3D::Vector3int16& translation,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const;
    const OrientedEdge& GetOrientedEdge (size_t i) const
    {
	return *m_orientedEdges[i];
    }
    boost::shared_ptr<OrientedEdge> GetOrientedEdgePtr (size_t i) const
    {
	return m_orientedEdges[i];
    }
    /**
     * Gets the list of oriented edges
     * @return vector of oriented edges
     */
    const OrientedEdges& GetOrientedEdges () const
    {
	return m_orientedEdges;
    }
    OrientedEdges& GetOrientedEdges ()
    {
	return m_orientedEdges;
    }

    bool IsClosed () const;
    size_t size () const
    {
	return m_orientedEdges.size ();
    }
    string ToString (const AttributesInfo* ai = 0) const;
    bool IsStandalone () const
    {
	return GetAdjacentBodySize () == 0;
    }
    void PrintAdjacentBodyInformation (ostream& ostr) const;
    void UpdateAdjacentFaceStandalone (boost::shared_ptr<Face> face);
    void CalculateCentroidAndArea ();
    void CalculatePerimeter ();
    QColor GetColor (const QColor& defaultColor) const;
    size_t GetEdgesPerFace (bool is2D) const;
    size_t GetConstraintIndex (size_t i = 0) const;
    bool HasConstraints () const;
    
private:
    boost::shared_ptr<Face> createDuplicate (
	const OOBox& periods, const G3D::Vector3& newBegin,
	VertexSet* vertexSet, EdgeSet* edgeSet) const;
    double getMaxEdgeLength ();
    /**
     * Calculate a orthogonal system, where XY is on the face and Z is normal
     * to the face
     */
    void calculateAxes (
	G3D::Vector3* x, G3D::Vector3* y, G3D::Vector3* z) const;
    size_t largestEdgeIndex () const;

private:
    /**
     * Edges that are part of this face
     */
    OrientedEdges m_orientedEdges;
    /**
     * Bodies this face is part of.
     */
    /**
     * Standalone faces need a place to store an OrientedFace
     * for the list of faces part of each edge.
     */
    boost::shared_ptr<OrientedFace> m_orientedFace;
    vector<AdjacentBody> m_adjacentBodies;
    G3D::Vector3 m_normal;
    G3D::Vector3 m_center;
    float m_perimeter;
    float m_area;
};
/**
 * Pretty prints this Face by printing the edges in DIRECT order
 */
inline ostream& operator<< (ostream& ostr, const Face& f)
{
    return ostr << f.ToString ();
}

/**
 * Pretty prints a boost::shared_ptr<Face> 
 * @param ostr where to print the Face object
 * @param f what to print
 * @return stream where to print other data
 */
inline ostream& operator<< (ostream& ostr,
			    const boost::shared_ptr<Face>& f)
{
    return ostr << *f << "useCount=" << f.use_count ();
}


#endif //__FACE_H__

// Local Variables:
// mode: c++
// End:
