/**
 * @file   OrientedEdge.h
 * @author Dan R. Lipsa
 * @brief An oriented edge. Allows using an Edge in direct or reversed order.
 * @ingroup data model
 */
#ifndef __ORIENTED_EDGE_H__
#define __ORIENTED_EDGE_H__

#include "AdjacentOrientedFace.h"
#include "OrientedElement.h"
class Edge;
class OrientedFace;
class Vertex;

/**
 * @brief An oriented edge. Allows using an Edge in direct or reversed order.
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
    const Vertex& GetBegin () const
    {
	return *GetBeginPtr ();
    }
    boost::shared_ptr<Vertex> GetBeginPtr () const;
    G3D::Vector3 GetBeginVector () const;
    
    /**
     * Get the end vertex of the OrientedEdge. Note that this might be the
     * begin vertex for the edge stored inside.
     */
    boost::shared_ptr<Vertex> GetEndPtr (void) const;
    const Vertex& GetEnd (void) const
    {
	return *GetEndPtr ();
    }
    G3D::Vector3 GetEndVector () const;

    /**
     * Adds a face that is touched by this oriented edge.
     */
    void AddAdjacentOrientedFace (boost::shared_ptr<OrientedFace> face, 
				  size_t edgeIndex) const;
    const AdjacentOrientedFaces& GetAdjacentOrientedFaces () const;
    size_t GetAdjacentOrientedFacesSize () const;
    bool HasConstraints () const;
    size_t GetConstraintIndex () const;

    /**
     * Edge for this oriented edge
     * @return the edge for this oriented edge
     */
    boost::shared_ptr<Edge> GetEdge () const;
    
    void SetEdge (boost::shared_ptr<Edge> edge);
    //bool IsZero () const;
    G3D::Vector3 GetEdgeVector () const;
    string ToString (const AttributesInfo* ai = 0) const;
    string ToStringShort () const;
    void GetVertexSet (VertexSet* vertexSet) const;
    bool IsPhysical (bool is2D) const;
    size_t GetPointCount () const;
    G3D::Vector3 GetPoint (size_t i) const;
    float GetLength () const;
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
