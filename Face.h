/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "ColoredElement.h"
#include "Comparisons.h"
#include "BodyIndex.h"

class AttributesInfo;
class Body;
class Foam;
class Edge;
class OrientedEdge;

/**
 * A Face is a oriented list of edges.
 */
class Face : public ColoredElement
{
public:
    typedef vector< boost::shared_ptr<OrientedEdge> > OrientedEdges;

public:
    /**
     * Constructs a Face object
     * @param id original index for this face
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face (const vector<int>& edgeIndexes,
	  const vector< boost::shared_ptr<Edge> >& edges, 
	  size_t id,
	  ElementStatus::Duplicate duplicateStatus = ElementStatus::ORIGINAL);
    Face (const Face& original);
    Face (const boost::shared_ptr<Edge>& firstEdge, size_t id);
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
    size_t GetEdgeCount () const
    {
	return m_orientedEdges.size ();
    }
    boost::shared_ptr<OrientedEdge> GetOrientedEdge (size_t i) const
    {
	return m_orientedEdges[i];
    }
    boost::shared_ptr<Edge>  GetEdge (size_t i) const;

    void AddBodyPartOf (boost::shared_ptr<Body>  body, size_t orientedFaceIndex)
    {
	m_bodiesPartOf.push_back (BodyIndex (body, orientedFaceIndex));
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
    void PrintBodyPartOfInformation (ostream& ostr) const;

    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;

    bool operator== (const Face& face) const;
    bool fuzzyEq (const Face& other) const;
    bool operator< (const Face& other) const;

    G3D::Vector3 GetNormal () const
    {
	return m_normal;
    }
    void CalculateNormal ();
    bool IsClosed () const;
    bool HasWrap () const;
    size_t size () const
    {
	return m_orientedEdges.size ();
    }
    void Unwrap (Foam* foam, VertexSet* vertexSet, EdgeSet* edgeSet);
    string ToString () const;
public:
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);
private:
    /**
     * Edges that are part of this face
     */
    OrientedEdges m_orientedEdges;
    vector<BodyIndex> m_bodiesPartOf;
    G3D::Vector3 m_normal;
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
