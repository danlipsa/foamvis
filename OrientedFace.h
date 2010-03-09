/**
 * @file OrientedFace.h
 * @author Dan R. Lipsa
 * 
 * Declaration for the OrientedFace class
 */
#ifndef __ORIENTED_FACE_H__
#define __ORIENTED_FACE_H__

#include "Face.h"

/**
 * An oriented face is a face  (list of edges) that can have its edges
 * read in direct or reversed order.
 */
class OrientedFace
{
public:
    /**
     * Constructs a OrientedFace object
     * @param face a pointer to a Face
     * @param reversed if true the edges in the Face should be read in
     *        reverse order,  otherwise they should be  read in direct
     *        order
     */
    OrientedFace(Face* face, bool reversed) : 
	m_face (face), m_reversed (reversed), m_placed (false)
    {}
    /**
     * Gets the face associated with this oriented face
     * @return the face associated with this oriented face
     */
    Face* GetFace () const
    {
	return m_face;
    }
    void AddAdjacentBody (Body* body) 
    {
	m_face->AddAdjacentBody (body);
    }
    vector<Body*>& GetAdjacentBodies ()
    {
	return m_face->GetAdjacentBodies ();
    }
    void UpdateEdgesAdjacency ()
    {
	m_face->UpdateEdgesAdjacency ();
    }
    void ClearEdgesAdjacency ()
    {
	m_face->ClearEdgesAdjacency ();
    }
    
    /**
     * Is this in the same order or reversed compared with the face associated
     * with it.
     * @return true for reversed, false otherwise
     */
    bool IsReversed () const
    {
	return m_reversed;
    }
    /**
     * Gets the begin vertex for an edge in this oriented face
     * @param edgeIndex what edge we are interested in
     * @return the begin vertex
     */
    Vertex* GetBegin (unsigned int edgeIndex) 
    {
	return getBegin (edgeIndex);
    }
    const Vertex* GetBegin (unsigned int edgeIndex) const
    {
	return getBegin (edgeIndex);
    }
    /**
     * Gets the end vertex for an edge in this oriented face
     * @param edgeIndex what edge we are interested in
     * @return the end vertex
     */
    Vertex* GetEnd (unsigned int edgeIndex) 
    {
	return getEnd (edgeIndex);
    }
    const Vertex* GetEnd (unsigned int edgeIndex) const
    {
	return getEnd (edgeIndex);
    }
    bool IsPlaced () const
    {
	return m_placed;
    }
    void SetPlaced (bool placed)
    {
	m_placed = placed;
    }

public:
    /**
     * Pretty print for the OrientedFace object
     */
    friend ostream& operator<< (ostream& ostr, const OrientedFace& of);

private:
    Vertex* getBegin (unsigned int edgeIndex) const;
    Vertex* getEnd (unsigned int edgeIndex) const;

private:
    /**
     * Object that has information about the edges in this OrientedFace
     */
    Face* m_face;
    /**
     * If true,  edges in the refering  Face object should  be read in
     * reversed order
     */
    bool m_reversed;
    bool m_placed;
};
/**
 * Pretty prints a pointer to an oriented face
 * @param ostr where to print
 * @param pof pointer to an oriented face
 * @return where to print next
 */
inline ostream& operator<< (ostream& ostr, const OrientedFace* pof)
{
    return ostr << *pof;
}


#endif //__ORIENTED_FACE_H__

// Local Variables:
// mode: c++
// End:
