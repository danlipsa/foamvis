/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "Element.h"
#include "Comparisons.h"
#include "BodyIndex.h"

class AttributesInfo;
class Body;
class ConstraintEdge;
class Foam;
class Edge;
class OrientedEdge;
class OOBox;

/**
 * A Face is a oriented list of edges.
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
    Face (const Face& original);
    Face (const boost::shared_ptr<Edge>& firstEdge, size_t id);


    void AddBodyPartOf (boost::shared_ptr<Body>  body, size_t orientedFaceIndex)
    {
	m_bodiesPartOf.push_back (BodyIndex (body, orientedFaceIndex));
    }
    void AddEdge (boost::shared_ptr<Edge> edge);
    void CalculateNormal ();
    const G3D::Vector3& GetCenter () const
    {
	return m_center;
    }
    size_t GetEdgeCount () const
    {
	return m_orientedEdges.size ();
    }

    const BodyIndex& GetBodyPartOf (bool faceReversed) const;
    const BodyIndex& GetBodyPartOf (size_t i) const
    {
	return m_bodiesPartOf[i];
    }
    size_t GetBodyPartOfSize () const
    {
	return m_bodiesPartOf.size ();
    }
    void GetVertexSet (VertexSet* vertexSet) const;
    void GetEdgeSet (EdgeSet* edgeSet) const;


    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;

    bool operator== (const Face& face) const;
    bool fuzzyEq (const Face& other) const;
    bool operator< (const Face& other) const;

    G3D::Vector3 GetNormal () const
    {
	return m_normal;
    }
    double GetPerimeter () const
    {
	return m_perimeter;
    }
    double GetArea () const
    {
	return m_area;
    }
    boost::shared_ptr<Face> GetDuplicate (
	const OOBox& periods, const G3D::Vector3int16& translation,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const;
    boost::shared_ptr<OrientedEdge> GetOrientedEdge (size_t i) const
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
    string ToString () const;
    bool IsStandalone () const
    {
	return GetBodyPartOfSize () == 0;
    }
    void PrintBodyPartOfInformation (ostream& ostr) const;
    void UpdateStandaloneFacePartOf (boost::shared_ptr<Face> face);
    void CalculateCentroidAndArea ();
    void CalculatePerimeter ();
    QColor GetColor (const QColor& defaultColor) const;

private:
    boost::shared_ptr<Face> createDuplicate (
	const OOBox& periods, const G3D::Vector3& newBegin,
	VertexSet* vertexSet, EdgeSet* edgeSet) const;
    double getMaxEdgeLength ();

private:
    /**
     * Edges that are part of this face
     */
    OrientedEdges m_orientedEdges;
    /**
     * Bodies this face is part of.
     */
    vector<BodyIndex> m_bodiesPartOf;
    G3D::Vector3 m_normal;
    G3D::Vector3 m_center;
    double m_perimeter;
    double m_area;
    /**
     * Standalone faces need a place to store an OrientedFace
     * for the list of faces part of each edge.
     */
    boost::shared_ptr<OrientedFace> m_orientedFace;
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
