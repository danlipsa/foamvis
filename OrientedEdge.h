/**
 * @file   OrientedEdge.h
 * @author Dan R. Lipsa
 *
 * Declaration of the OrientedEdge object
 */
#ifndef __ORIENTED_EDGE_H__
#define __ORIENTED_EDGE_H__

#include "OrientedFaceIndex.h"
#include "OrientedElement.h"
class Edge;
class OrientedFace;
class Vertex;

/**
 * An oriented  edge is  an edge  that can have  its vertices  read in
 * direct and reverse order
 */
class OrientedEdge : public OrientedElement
{
public:
    OrientedEdge () :
	OrientedElement () 
    {
    }

    /**
     * Constructs an oriented edge
     * @param edge information about the two vertices in the oriented edge
     * 
     * @param  reversed specifies  if  the two  vertices  are read  in
     *        reverse order or not.
     */
    OrientedEdge (const boost::shared_ptr<Edge>& edge, bool reversed);
    /**
     * Get the begin vertex of the OrientedEdge. Note that this might be the 
     * end vertex for the edge stored inside.
     * @return the begin vertex
     */
    boost::shared_ptr<Vertex> GetBegin (void) const;
    
    /**
     * Get the end vertex of the OrientedEdge. Note that this might be the
     * begin vertex for the edge stored inside.
     */
    boost::shared_ptr<Vertex> GetEnd (void) const;
    /**
     * Adds a face that is touched by this oriented edge.
     */
    void AddFacePartOf (boost::shared_ptr<OrientedFace> face, 
			size_t edgeIndex) const;
    size_t GetFacePartOfSize () const;
    OrientedFaceIndexList::const_iterator GetFacePartOfBegin () const;
    OrientedFaceIndexList::const_iterator GetFacePartOfEnd () const;

    /**
     * Edge for this oriented edge
     * @return the edge for this oriented edge
     */
    boost::shared_ptr<Edge> GetEdge () const;
    
    void SetEdge (boost::shared_ptr<Edge> edge);
    bool IsZero () const;
    G3D::Vector3 GetEdgeVector () const;
    ostream& PrintReversed (ostream& ostr) const
    {
	return print (ostr, true);
    }
    string ToString () const;

private:
    ostream& print (ostream& ostr, bool reversed = false) const;
};
/**
 * Pretty prints an Edge
 * @param ostr output stream where to print the edge
 * @param oe the edge to by printed.
 */
inline ostream& operator<< (ostream& ostr, const OrientedEdge& oe)
{
    return ostr << oe.ToString ();
}

/**
 * Pretty prints an oriented edge pointer
 * @param ostr where to print
 * @param poe pointer to the oriented edge
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, boost::shared_ptr<OrientedEdge> poe)
{
    return ostr << *poe;
}

#endif //__ORIENTED_EDGE_H__

// Local Variables:
// mode: c++
// End:
