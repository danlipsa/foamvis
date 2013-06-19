/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * @brief Part of a bubble face, stores a begin and end vertex.
 * @ingroup data
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
 * @brief Part of a bubble face, stores a begin and end vertex.
 */
class Edge : public Element
{
public:
    enum Type
    {
	EDGE,
	QUADRATIC_EDGE,
	CONSTRAINT_EDGE
    };

    Edge (const boost::shared_ptr<Vertex>& begin,
	  const boost::shared_ptr<Vertex>& end, 
	  const G3D::Vector3int16& endLocation, 
	  size_t id, Type type = EDGE,
	  ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    Edge (const boost::shared_ptr<Vertex>& begin, size_t id, Type type = EDGE);
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
     *
     * 3D:
     * A physical edge has 3 faces adjacent to it 
     * (or 6 oriented faces or 4 oriented faces for outside faces).
     *
     * 2D:
     * In quadradic model all edges are physical
     */
    bool IsPhysical (bool is2D) const;

    /**
     * Adds a face touched by this edge
     */
    void AddAdjacentOrientedFace (
	boost::shared_ptr<OrientedFace>  face, size_t edgeIndex);
    const AdjacentOrientedFaces& GetAdjacentOrientedFaces () const
    {
	return m_adjacentOrientedFaces;
    }
    size_t GetAdjacentOrientedFacesSize () const
    {
	return m_adjacentOrientedFaces.size ();
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

    bool IsStandalone () const
    {
	return GetAdjacentOrientedFaces ().size () == 0;
    }

    size_t GetConstraintIndex (size_t i = 0) const;
    bool HasConstraints () const;

    string ToString (const AttributesInfo* ai = 0) const;
    void GetVertexSet (VertexSet* vertexSet) const;
    float GetLength () const;

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
    Type GetType () const
    {
	return m_type;
    }
    bool IsQuadratic () const
    {
        return GetType () == QUADRATIC_EDGE;
    }

public:
    static short LocationCharToNumber (char sign);    
    static G3D::Vector3int16 IntToLocation (int i);
    /**
     * Domain increment can be *, - or +
     */
    static const size_t DOMAIN_INCREMENT_POSSIBILITIES = 3;


protected:
    Edge (const Edge& edge);
    virtual boost::shared_ptr<Edge> createDuplicate (
	const OOBox& originalDomain,
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
    Type m_type;
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
