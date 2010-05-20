/**
 * @file   OrientedEdge.h
 * @author Dan R. Lipsa
 *
 * Declaration of the OrientedEdge object
 */
#ifndef __ORIENTED_EDGE_H__
#define __ORIENTED_EDGE_H__

#include "OrientedFaceIndex.h"
class Edge;
class Face;
class OrientedFace;
class Vertex;

/**
 * An oriented  edge is  an edge  that can have  its vertices  read in
 * direct and reverse order
 */
class OrientedEdge
{
public:
    OrientedEdge () : m_edge (0), m_reversed (false) {}
    /**
     * Constructs an oriented edge
     * @param edge information about the two vertices in the oriented edge
     * 
     * @param  reversed specifies  if  the two  vertices  are read  in
     *        reverse order or not.
     */
    OrientedEdge (Edge* edge, bool reversed): 
        m_edge (edge), m_reversed (reversed) {}
    /**
     * Get the begin vertex of the OrientedEdge. Note that this might be the 
     * end vertex for the edge stored inside.
     * @return the begin vertex
     */
    Vertex* GetBegin (void) const;

    bool IsReversed () const {return m_reversed;}
    void Reverse ()
    {
	m_reversed = ! m_reversed;
    }
    size_t GetId () const;
    int GetSignedId () const
    {
	return IsReversed () ? (- GetId ()) : GetId ();
    }
    
    /**
     * Get the end vertex of the OrientedEdge. Note that this might be the
     * begin vertex for the edge stored inside.
     */
    Vertex* GetEnd (void) const;
    /**
     * Adds a face that is touched by this oriented edge.
     */
    void AddFacePartOf (Face* face, bool faceReversed, size_t edgeIndex);
    void ClearFacePartOf ();
    size_t GetFacePartOfSize () const;
    const OrientedFaceIndex& GetFacePartOf (size_t i) const;

    /**
     * Edge for this oriented edge
     * @return the edge for this oriented edge
     */
    Edge* GetEdge () const 
    {
	return m_edge;
    }
    
    void SetEdge (Edge* edge) 
    {
	m_edge = edge;
    }
    bool IsZero () const;
    G3D::Vector3 GetEdgeVector () const;
    ostream& PrintReversed (ostream& ostr) const
    {
	return print (ostr, true);
    }
    /**
     * Calculate the translation from a source oriented edge to this edge
     * @param source source oriented edge
     * @param translation vector to translate the source edge to this edge
     */
    void CalculateTranslation (const OrientedEdge& source, 
			       G3D::Vector3* translation = 0) const;

public:
    /**
     * Pretty prints an Edge
     * @param ostr output stream where to print the edge
     * @param oe the edge to by printed.
     */
    friend ostream& operator<< (ostream& ostr, const OrientedEdge& oe)
    {
	return oe.print (ostr);
    }

private:
    ostream& print (ostream& ostr, bool reversed = false) const;

private:
    /**
     * Stores information about the two vertices in this oriented edge
     */
    Edge* m_edge;
    /**
     * Specifies if the  two vertices should be read  in reverse order
     * or in direct order.
     */
    bool m_reversed;
};

/**
 * Pretty prints an oriented edge pointer
 * @param ostr where to print
 * @param poe pointer to the oriented edge
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, OrientedEdge* poe)
{
    return ostr << *poe;
}

#endif //__ORIENTED_EDGE_H__

// Local Variables:
// mode: c++
// End:
