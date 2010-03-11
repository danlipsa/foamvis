/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "OrientedFace.h"
#include "Debug.h"


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
