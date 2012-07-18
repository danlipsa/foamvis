/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display a vertex
 */

#ifndef __DISPLAY_VERTEX_FUNCTORS_H__
#define __DISPLAY_VERTEX_FUNCTORS_H__

#include "DisplayElement.h"
class Edge;
class OrientedEdge;
class Vertex;
class Settings;

void DisplayEdgeVertices (const Edge& edge,
			  bool useZPos = false, double zPos = 0);
void DisplayEdgeVerticesNoEnds (const Edge& edge);
void  DisplayOrientedEdgeVertices (const boost::shared_ptr<OrientedEdge> oe);
struct DisplayOriginalVertex
{
    void operator() (const boost::shared_ptr<Vertex>& v);
};

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
struct DisplayBeginVertex
{
    DisplayBeginVertex () 
    {
    }
    
    DisplayBeginVertex (const Settings&) 
    {
    }
    
    void operator() (const boost::shared_ptr<const OrientedEdge> oe)
    {
	operator () (oe.get ());
    }
    void operator () (const OrientedEdge* oe);
    void operator () (const OrientedEdge& oe)
    {
	operator () (&oe);
    }
};

class DisplayTriangle : 
    public unary_function <const boost::shared_ptr<OrientedEdge>, void>
{
public:
    DisplayTriangle (const G3D::Vector3& center) :
	m_center (center)
    {
    }

    void operator() (const boost::shared_ptr<OrientedEdge> e) const;
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end) const;
private:
    const G3D::Vector3& m_center;
};


#endif //__DISPLAY_VERTEX_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
