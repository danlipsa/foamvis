/**
 * @file OrientedFace.h
 * @author Dan R. Lipsa
 * 
 * Declaration for the OrientedFace class
 */
#ifndef __ORIENTED_FACE_H__
#define __ORIENTED_FACE_H__

class Body;
class Face;
class Vertex;
class OrientedEdge;

/**
 * An oriented face is a face  (list of edges) that can have its edges
 * read in direct or reversed order. If 0, 1, 2, ..., n-1 are the
 * edges of the face in direct order, n-1, n-2, ..., 1, 0 are the
 * edges of the face in reverse order.
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
    void AddAdjacentBody (Body* body);
    Body* GetAdjacentBody ();
    void UpdateEdgeAdjacency ();
    void ClearEdgeAdjacency ();
    
    /**
     * Is this in the same order or reversed compared with the face associated
     * with it.
     * @return true for reversed, false otherwise
     */
    bool IsReversed () const
    {
	return m_reversed;
    }
    size_t GetId () const;
    int GetSignedId () const
    {
	return IsReversed () ? (- GetId ()) : GetId ();
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
    void GetOrientedEdge (size_t edgeIndex, OrientedEdge* oe) const;

    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;
    G3D::Vector3 GetNormal () const;

    size_t size () const;
    void CalculateTranslation (
	const OrientedEdge& edge, G3D::Vector3* translation) const;
    

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

    Vertex* getBegin (size_t edgeIndex) const;
    Vertex* getEnd (size_t edgeIndex) const;

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
