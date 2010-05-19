/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "Debug.h"
#include "Edge.h"
#include "Vertex.h"

ostream& OrientedEdge::print (ostream& ostr, bool reversed) const
{
    using G3D::Vector3;
    ostr << (m_reversed ? "(R)" : "(N)");
    ostr << "Oriented Edge " << GetEdge ()-> GetId () << " "
	 << GetEdge ()->GetStatus ()
	 << ": ";
    const Vector3* begin = static_cast<const G3D::Vector3*>(GetBegin ());
    const Vector3* end = static_cast<const G3D::Vector3*>(GetEnd ());
    if (reversed)
	swap (begin, end);
    ostr << *begin << ", " << *end;
    return ostr;
}

bool OrientedEdge::Fits (const OrientedEdge& destination,
			 G3D::Vector3* translation) const
{
    if (GetEdge ()->GetId () == 
	destination.GetEdge ()->GetId () &&
	IsReversed () == ! destination.IsReversed ())
    {
	if (translation != 0)
	    *translation = *destination.GetEnd () - *GetBegin ();
	return true;
    }
    else
	return false;
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
