/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__

#include "ColoredElement.h"
#include "OrientedFaceIndex.h"
class AttributesInfo;
class Foam;
class OOBox;
class OrientedFace;
class Vertex;

/**
 * An edge is an object that stores a begin and an end vertex
 */
class Edge : public ColoredElement
{
public:
    /**
     * Creates an Edge object
     * @param id what is the original index for this edge
     * @param begin the first point of the endge
     * @param end the last point of the edge
     */
    Edge (const boost::shared_ptr<Vertex>& begin,
	  const boost::shared_ptr<Vertex>& end, 
	  const G3D::Vector3int16& endLocation, 
	  size_t id,
	  ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    Edge (const boost::shared_ptr<Vertex>& begin, size_t id);
    Edge (const Edge& edge);
    /**
     * @return the first vertex of the edge
     */
    boost::shared_ptr<Vertex> GetBegin() const 
    {
	return m_begin;
    }

    /**
     * Translate the begin vertex so that the end vertex coincides with 
     * the 'end' parameter.
     */
    G3D::Vector3 GetTranslatedBegin (const G3D::Vector3& newEnd) const;
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void SetBegin(boost::shared_ptr<Vertex> begin) 
    {
	m_begin = begin;
    }
    /**
     * @return last vertex of the edge
     */
    boost::shared_ptr<Vertex> GetEnd() const 
    {
	return m_end;
    }
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    void SetEnd(boost::shared_ptr<Vertex> end) 
    {
	m_end = end;
    }
    /**
     * Prints the two vertices of an edge in reverse order (end , begin)
     * @param ostr the stream where to write the edge
     */
    void ReversePrint (ostream& ostr) const;
    /**
     * Is this a physical edge (not a tesselation edge)?
     * 3D:
     * A physical edge has 3 faces adjacent to it 
     * (or 6 oriented faces or 4 oriented faces for outside faces).
     * 2D:
     * In quadradic model all edges are physical
     * @return true if this is a physical edge, false otherwise
     */
    bool IsPhysical (size_t dimension, bool isQuadratic) const;

    /**
     * Adds a face touched by this edge
     * @param face face adjacent to this edge
     * @param reversed the edge is reversed in the face list
     * @param
     */
    void AddFacePartOf (boost::shared_ptr<OrientedFace>  face, size_t edgeIndex);
    size_t GetFacePartOfSize () const
    {
	return m_facesPartOf.size ();
    }
    OrientedFaceIndexList::const_iterator GetFacePartOfBegin () const
    {
	return m_facesPartOf.begin ();
    }
    OrientedFaceIndexList::const_iterator GetFacePartOfEnd () const
    {
	return m_facesPartOf.end ();
    }
    void PrintFacePartOfInformation (ostream& ostr) const;


    /**
     * For both  vertices of this edge,  add the edge as  being adjacent to
     * the vertices
     */
    void UpdateEdgePartOf (const boost::shared_ptr<Edge>& edge);
    const G3D::Vector3int16& GetEndTranslation () const
    {
	return m_endTranslation;
    }
    bool operator== (const Edge& other) const;
    bool fuzzyEq (const Edge& other) const;
    bool operator< (const Edge& other) const;
    bool IsZero () const;

    G3D::Vector3 GetTorusClippedBegin (size_t index) const;
    G3D::Vector3 GetTorusClippedEnd (size_t index) const;
    size_t GetTorusClippedSize (const OOBox& periods) const;
    void CalculateTorusClipped (const OOBox& periods);
    bool IsClipped () const
    {
	ElementStatus::Enum duplicateStatus = GetDuplicateStatus ();
	return (duplicateStatus != ElementStatus::DUPLICATE);
    }

    bool IsStandalone () const
    {
	return GetFacePartOfSize () == 0;
    }

    string ToString () const;
    void GetVertexSet (VertexSet* vertexSet) const;

public:
    static short LocationCharToNumber (char sign);
    static G3D::Vector3int16 IntToLocation (int i);

private:
    /**
     * First vertex of the edge
     */
    boost::shared_ptr<Vertex> m_begin;
    /**
     * Last vertex of the edge
     */
    boost::shared_ptr<Vertex> m_end;
    G3D::Vector3int16 m_endTranslation;
    /**
     * Stores adjacent faces to this edge. Assume no duplicate edges
     * in 3D torus model.
     */
    OrientedFaceIndexList m_facesPartOf;
    boost::scoped_ptr< vector<G3D::LineSegment> > m_torusClipped;

};
/**
 * Prints an edge to the output stream
 * @param ostr where to write the edge
 * @param e edge to write
 */
inline ostream& operator<< (ostream& ostr, const Edge& e)
{
    return ostr << e.ToString ();
}

/**
 * Pretty print an boost::shared_ptr<Edge> 
 * @param ostr where to print
 * @param e what to print
 * @return where to print the next data
 */
inline ostream& operator<< (ostream& ostr, const boost::shared_ptr<Edge>& e)
{
    return ostr << *e << "useCount=" << e.use_count ();;
}

#endif //__EDGE_H__

// Local Variables:
// mode: c++
// End:
