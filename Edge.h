/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__

#include "Element.h"
#include "AdjacentOrientedFace.h"
class AttributesInfo;
class Foam;
class OOBox;
class OrientedFace;
class Vertex;

/**
 * An edge is an object that stores a begin and an end vertex
 */
class Edge : public Element
{
public:
    /**
     * Creates an Edge object
     * @param begin the first point of the endge
     * @param end the last point of the edge
     * @param endLocation used in torus model to mark a translation relative to
     *        the domain where the second vertex is defined in the data file.
     * @param id what is the original index for this edge
     * @param duplicateStatus is this an original edge or a duplicate
     */
    Edge (const boost::shared_ptr<Vertex>& begin,
	  const boost::shared_ptr<Vertex>& end, 
	  const G3D::Vector3int16& endLocation, 
	  size_t id,
	  ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    Edge (const boost::shared_ptr<Vertex>& begin, size_t id);
    virtual ~Edge ()
    {
    }
    /**
     * @return the first vertex of the edge
     */
    const Vertex& GetBegin () const 
    {
	return *m_begin;
    }
    boost::shared_ptr<Vertex> GetBeginPtr () const 
    {
	return m_begin;
    }
    G3D::Vector3 GetBeginVector () const;

    /**
     * Translate the begin vertex so that the end vertex coincides with 
     * the 'end' parameter.
     */
    G3D::Vector3 GetTranslatedBegin (const G3D::Vector3& newEnd) const;
    /**
     * @return last vertex of the edge
     */
    const Vertex& GetEnd () const 
    {
	return *m_end;
    }
    G3D::Vector3 GetEndVector () const;
    
    boost::shared_ptr<Vertex> GetEndPtr () const 
    {
	return m_end;
    }
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    virtual void SetEnd(boost::shared_ptr<Vertex> end) 
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
    bool IsPhysical (bool foam2D, bool isQuadratic) const;

    /**
     * Adds a face touched by this edge
     */
    void AddAdjacentFace (
	boost::shared_ptr<OrientedFace>  face, size_t edgeIndex);
    const AdjacentOrientedFaces& GetAdjacentFaces () const
    {
	return m_adjacentOrientedFaces;
    }
    string AdjacentFacesToString () const;


    /**
     * For both  vertices of this edge,  add the edge as  being adjacent to
     * the vertices
     */
    void UpdateAdjacentEdge (const boost::shared_ptr<Edge>& edge);
    const G3D::Vector3int16& GetEndTranslation () const
    {
	return m_endTranslation;
    }
    bool operator== (const Edge& other) const;
    bool fuzzyEq (const Edge& other) const;
    bool operator< (const Edge& other) const;

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
	return GetAdjacentFaces ().size () == 0;
    }

    size_t GetConstraintIndex (size_t i) const;
    size_t GetConstraintIndex () const
    {
	return GetConstraintIndex (0);
    }
    bool HasConstraints () const;

    string ToString () const;
    void GetVertexSet (VertexSet* vertexSet) const;
    double GetLength () const;

    virtual boost::shared_ptr<Edge> Clone () const;
    boost::shared_ptr<Edge> GetDuplicate (
	const OOBox& periods,
	const G3D::Vector3& newBegin,
	VertexSet* vertexSet, EdgeSet* edgeSet) const;    

    virtual size_t GetPointCount () const
    {
	return 2;
    }
    virtual G3D::Vector3 GetPoint (size_t i) const;
    QColor GetColor (const QColor& defaultColor) const;

public:
    static short LocationCharToNumber (char sign);    
    static G3D::Vector3int16 IntToLocation (int i);
    /**
     * Domain increment can be *, - or +
     */
    static const size_t DOMAIN_INCREMENT_POSSIBILITIES = 3;


protected:
    Edge (const Edge& edge);

protected:
    virtual boost::shared_ptr<Edge> createDuplicate (
	const OOBox& periods,
	const G3D::Vector3& newBegin, VertexSet* vertexSet) const;

private:
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void setBegin(boost::shared_ptr<Vertex> begin) 
    {
	m_begin = begin;
    }


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
    AdjacentOrientedFaces m_adjacentOrientedFaces;
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
