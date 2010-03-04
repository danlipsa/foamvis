/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "OrientedFace.h"
#include "Debug.h"


ostream& operator<< (ostream& ostr, OrientedEdge& oe)
{
    ostr << (oe.m_reversed ? "(R)" : "(N)");
    ostr << "Oriented Edge " << oe.GetEdge ()-> GetOriginalIndex () 
	 << (oe.GetEdge ()->IsDuplicate () ? " DUPLICATE" : "")
	 << ": "
	 << static_cast<G3D::Vector3>(*oe.GetBegin ()) << ", " 
	 << static_cast<G3D::Vector3>(*oe.GetEnd ());
    return ostr;
}

G3D::Vector3int16 OrientedEdge::GetEndDomainIncrement ()
{
    if (m_reversed)
	return G3D::Vector3int16 (0,0,0) - m_edge->GetEndDomainIncrement ();
    else
	return m_edge->GetEndDomainIncrement ();;
}
