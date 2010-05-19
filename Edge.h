/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__

#include "ColoredElement.h"
class Vertex;
class AttributesInfo;
class Face;
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
    Edge (Vertex* begin, Vertex* end, 
	  G3D::Vector3int16& endLocation, 
	  size_t id, Foam* data,
	  ElementStatus::Name status = ElementStatus::ORIGINAL);
    Edge (Vertex* begin, size_t id);
    Edge (const Edge& edge);
    /**
     * @return the first vertex of the edge
     */
    Vertex* GetBegin() const 
    {
	return m_begin;
    }

    G3D::Vector3 GetBegin (const G3D::Vector3* end) const;
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void SetBegin(Vertex* begin) 
    {
	m_begin = begin;
    }
    /**
     * @return last vertex of the edge
     */
    Vertex* GetEnd() const 
    {
	return m_end;
    }
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    void SetEnd(Vertex* end) 
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
     * @return true if this is a physical edge, false otherwise
     */
    bool IsPhysical () const
    {
	return m_physical || (m_adjacentFaces.size () == 3);
    }
    void SetPhysical (bool physical)
    {
	m_physical = physical;
    }

    /**
     * Adds a face touched by this edge
     * @param face face touched by this edge
     */
    void AddAdjacentFace (Face* face)
    {
	m_adjacentFaces.push_back (face);
    }
    void ClearAdjacentFaces ()
    {
	m_adjacentFaces.clear ();
    }
    const vector<Face*>& GetAdjacentFaces () const
    {
	return m_adjacentFaces;
    }
    /**
     * For both  vertices of this edge,  add the edge as  being adjacent to
     * the vertices
     */
    void UpdateVertexAdjacency ();
    const G3D::Vector3int16& GetEndTranslation () const
    {
	return m_endTranslation;
    }
    bool operator== (const Edge& other) const;
    bool fuzzyEq (const Edge& other) const;
    bool operator< (const Edge& other) const;
    bool IsZero () const;
    Edge* CreateDuplicate (const G3D::Vector3& newBegin);
    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Edge::m_infos);
    }

    G3D::Vector3 GetTorusClippedBegin (size_t index) const;
    G3D::Vector3 GetTorusClippedEnd (size_t index) const;
    size_t GetTorusClippedSize () const;
    void CalculateTorusClipped ();
    bool IsClipped () const
    {
	ElementStatus::Name status = GetStatus ();
	return (status == ElementStatus::ORIGINAL || 
		status == ElementStatus::DUPLICATE_MADE);
    }

    bool ShouldDisplay () const
    {
	return GetAdjacentFaces ().size () != 0 ||
	    GetStatus () == ElementStatus::ORIGINAL;
    }
    
public:
    static short LocationCharToNumber (char sign);
    static G3D::Vector3int16 IntToLocation (int i);
    /**
     * Specifies the default attributes for an Edge object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);
    /**
     * Prints an edge to the output stream
     * @param ostr where to write the edge
     * @param e edge to write
     */
    friend ostream& operator<< (ostream& ostr, const Edge& e);

private:
    /**
     * First vertex of the edge
     */
    Vertex* m_begin;
    /**
     * Last vertex of the edge
     */
    Vertex* m_end;
    G3D::Vector3int16 m_endTranslation;
    /**
     * Stores adjacent faces to this edge. Assume no duplicate edges
     * in 3D torus model.
     */
    vector<Face*> m_adjacentFaces;
    bool m_physical;
    boost::scoped_ptr< vector<G3D::LineSegment> > m_torusClipped;

private:
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty print an Edge*
 * @param ostr where to print
 * @param e what to print
 * @return where to print the next data
 */
inline ostream& operator<< (ostream& ostr, const Edge* e)
{
    return ostr << *e;
}

#endif //__EDGE_H__

// Local Variables:
// mode: c++
// End:
