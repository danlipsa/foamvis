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
    if (oe.m_reversed)
        oe.m_edge->ReversePrint (ostr);
    else
        ostr << *oe.m_edge;
    return ostr;
}


G3D::Vector3int16 OrientedEdge::GetEndDomainIncrement ()
{
    if (m_reversed)
	return Vector3int16 (0,0,0) - m_edge->GetEndDomainIncrement ();
    else
	return m_edge->GetEndDomainIncrement ();;
}
