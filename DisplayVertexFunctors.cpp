/**
 * @file   DisplayVertexFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display a vertex
 */

#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "OrientedEdge.h"
#include "Vertex.h"
#include "OpenGLUtils.h"


// DisplayOriginalVertex
// ======================================================================

void DisplayOriginalVertex::operator() (const boost::shared_ptr<Vertex>& v)
{
    if (v->GetDuplicateStatus () != ElementStatus::DUPLICATE)
    {
	::glVertex (v->GetVector ());	
    }
}

// DisplayBeginVertex
// ======================================================================

void DisplayBeginVertex::operator() (const OrientedEdge* e)
{
    ::glVertex (e->GetBeginVector ());
}

// DisplayTriangle
// ======================================================================

void DisplayTriangle::operator() (const boost::shared_ptr<OrientedEdge> e) const
{
    operator () (e->GetBeginVector (), e->GetEndVector ());
}

void DisplayTriangle::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end) const
{
    ::glVertex (m_center);
    ::glVertex (begin);
    ::glVertex (end);
}
