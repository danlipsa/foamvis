/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "OrientedFace.h"
#include "Debug.h"
#include "Edge.h"
#include "Vertex.h"

ostream& OrientedEdge::print (ostream& ostr, bool reversed) const
{
    using G3D::Vector3;
    ostr << (m_reversed ? "(R)" : "(N)");
    ostr << "Oriented Edge " << GetEdge ()-> GetOriginalIndex () 
	 << (GetEdge ()->IsDuplicate () ? " DUPLICATE" : "")
	 << ": ";
    const Vector3* begin = static_cast<const G3D::Vector3*>(GetBegin ());
    const Vector3* end = static_cast<const G3D::Vector3*>(GetEnd ());
    if (reversed)
	swap (begin, end);
    ostr << *begin << ", " << *end;
    return ostr;
}


G3D::Vector3int16 OrientedEdge::GetEndDomainIncrement () const
{
    if (m_reversed)
	return G3D::Vector3int16 (0,0,0) - m_edge->GetEndDomainIncrement ();
    else
	return m_edge->GetEndDomainIncrement ();;
}

bool OrientedEdge::Fits (const OrientedEdge& other) const
{
    return 
	GetEdge ()->GetOriginalIndex () == 
	other.GetEdge ()->GetOriginalIndex () &&
	IsReversed () == ! other.IsReversed ();
}

G3D::Vector3 OrientedEdge::GetEdgeVector () const
{
    return *GetEnd () - *GetBegin ();
}

Vertex* OrientedEdge::GetBegin (void) const
{
    return m_reversed ? m_edge->GetEnd () : m_edge->GetBegin ();
}

Vertex* OrientedEdge::GetEnd (void) const
{
    return m_reversed ? m_edge->GetBegin () : m_edge->GetEnd ();
}

void OrientedEdge::AddAdjacentFace (Face* face) 
{
    m_edge->AddAdjacentFace (face);
}

void OrientedEdge::ClearAdjacentFaces ()
{
    m_edge->ClearAdjacentFaces ();
}

bool OrientedEdge::IsZero () const
{
    return m_edge->IsZero ();
}
