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
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "GLWidget.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "Vertex.h"

void DisplayEdgeVertices (const OrientedEdge& edge);

void DisplayEdgeVertices (const Edge& edge,
			  bool useZPos = false, double zPos = 0);


struct DisplayOriginalVertex
{
    void operator() (const boost::shared_ptr<Vertex>& v)
    {
	if (v->GetDuplicateStatus () != ElementStatus::DUPLICATE)
	{
	    glVertex (*v);	
	}
    }
};

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
struct DisplayBeginVertex
{
    DisplayBeginVertex () {}
    DisplayBeginVertex (const GLWidget&) {}
    void operator() (const boost::shared_ptr<OrientedEdge> e)
    {
	glVertex (*e->GetBegin ());
	//for (size_t i = 0; i < e->PointCount (); ++i)
	//glVertex(e->GetPoint (i));
	
    }
};

struct DisplayAllButLastVertices
{
    DisplayAllButLastVertices () 
    {}
    DisplayAllButLastVertices (const GLWidget&) 
    {}
    void operator() (const boost::shared_ptr<OrientedEdge> e)
    {
	for (size_t i = 0; i < e->PointCount () - 1; ++i)
	    glVertex(e->GetPoint (i));
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
    void operator() (const boost::shared_ptr<OrientedEdge> e) const
    {
	operator () (*e->GetBegin (), *e->GetEnd ());
    }
    void operator() (const G3D::Vector3& begin, const G3D::Vector3& end) const
    {
	glVertex (m_center);
	glVertex (begin);
	glVertex (end);
    }
private:
    const G3D::Vector3& m_center;
};


#endif //__DISPLAY_VERTEX_FUNCTORS_H__

// Local Variables:
// mode: c++
// End:
