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
    template <typename T>
    class Iterator : 
	boost::input_iterator_helper < Iterator<T>, OrientedEdge, 
				       Face::OrientedEdges::pointer,
				       Face::OrientedEdges::reference>
    {
    public:
	Iterator (T* of, size_t index) : m_of (of), m_index (index)
	{}
	friend bool operator==(Iterator first, Iterator second)
	{
	    return first.m_of == second.m_of && 
		first.m_index == second.m_index;
	}
	Iterator& operator++ ()
	{
	    m_index++;
	    return *this;
	}
	OrientedEdge operator* ()
	{
	    return m_of->GetOrientedEdge (m_index);
	}
    private:
	T* m_of;
	size_t m_index;
    };
    typedef Iterator<OrientedFace> iterator;
    typedef Iterator<const OrientedFace> const_iterator;

public:
    /**
     * Constructs a OrientedFace object
     * @param face a pointer to a Face
     * @param reversed if true the edges in the Face should be read in
     *        reverse order,  otherwise they should be  read in direct
     *        order
     */
    OrientedFace(Face* face, bool reversed) : 
	m_face (face), m_reversed (reversed)
    {}
    /**
     * Gets the face associated with this oriented face
     * @return the face associated with this oriented face
     */
    Face* GetFace () const
    {
	return m_face;
    }
    void SetFace (Face* face)
    {
	m_face = face;
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
    Vertex* GetBegin (size_t edgeIndex) 
    {
	return getBegin (edgeIndex);
    }
    const Vertex* GetBegin (size_t edgeIndex) const
    {
	return getBegin (edgeIndex);
    }
    /**
     * Gets the end vertex for an edge in this oriented face
     * @param edgeIndex what edge we are interested in
     * @return the end vertex
     */
    Vertex* GetEnd (size_t edgeIndex) 
    {
	return getEnd (edgeIndex);
    }
    const Vertex* GetEnd (size_t edgeIndex) const
    {
	return getEnd (edgeIndex);
    }

    /**
     * Gets the oriented edge at edgeIndex in face order.
     */
    OrientedEdge GetOrientedEdge (size_t edgeIndex) const;

    size_t GetNextValidIndex (size_t index) const
    {
	return m_face->GetNextValidIndex (index);
    }
    size_t GetPreviousValidIndex (size_t index) const
    {
	return m_face->GetPreviousValidIndex (index);
    }
    G3D::Vector3 GetNormal () const;

    size_t size () const
    {
	return m_face->GetEdgeCount ();
    }
    iterator begin ()
    {
	return Iterator<OrientedFace> (this, 0);
    }
    const_iterator begin () const
    {
	return Iterator<const OrientedFace> (this, 0);
    }

    iterator end ()
    {
	return Iterator<OrientedFace> (this, size ());
    }
    const_iterator end () const
    {
	return Iterator<const OrientedFace> (this, size ());
    }

public:
    /**
     * Pretty print for the OrientedFace object
     */
    friend ostream& operator<< (ostream& ostr, const OrientedFace& of);
    static pair<G3D::Vector3, OrientedFace*> MakeNormalFacePair (
	OrientedFace* face)
    {
	return pair<G3D::Vector3, OrientedFace*> (face->GetNormal (), face);
    }


private:

    Vertex* getBegin (size_t edgeIndex) const
    {
	return GetOrientedEdge (edgeIndex).GetBegin ();
    }

    Vertex* getEnd (size_t edgeIndex) const
    {
	return GetOrientedEdge (edgeIndex).GetEnd ();
    }

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
