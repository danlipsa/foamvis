/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "ColoredElement.h"
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
    typedef vector<OrientedEdge*> OrientedEdges;

public:
    /**
     * Constructs a Face object
     * @param id original index for this face
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face (vector<int>& edgeIndexes, vector<Edge*>& edges, 
	 size_t id, 
	 ElementStatus::Name status = ElementStatus::ORIGINAL);
    Face (const Face& original);
    Face (Edge* edge, size_t id);

    /**
     * Destroys a Face object
     */
    ~Face ();
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
    OrientedEdge* GetOrientedEdge (size_t i) const
    {
	return m_orientedEdges[i];
    }
    Edge* GetEdge (size_t i) const;

    void AddBodyPartOf (Body* body, size_t orientedFaceIndex)
    {
	m_bodiesPartOf.push_back (BodyIndex (body, orientedFaceIndex));
    }
    const BodyIndex& GetBodyPartOf (bool faceReversed) const;
    void ClearBodyPartOf ();
    void ClearFacePartOf ();


    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;

    bool operator== (const Face& face) const;
    G3D::Vector3 GetNormal () const
    {
	return m_normal;
    }
    void CalculateNormal ();
    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Face::m_infos);
    }
    bool IsClosed () const;
    bool HasWrap () const;
    size_t size () const
    {
	return m_orientedEdges.size ();
    }
    void Unwrap (Foam& foam);

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
    OrientedEdges m_orientedEdges;
    vector<BodyIndex> m_bodiesPartOf;
    G3D::Vector3 m_normal;
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
