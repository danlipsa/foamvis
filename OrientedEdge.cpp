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

void OrientedEdge::CalculateDomains ()
{
    Vertex* b = GetBegin ();
    Vector3int16 bDomain = b->GetDomain ();
    if (bDomain == Vertex::INVALID_DOMAIN)
	return;
    Vertex* e = GetEnd ();
    Vector3int16 eDomain = e->GetDomain ();
    if (eDomain == Vertex::INVALID_DOMAIN)
    {
	Vector3int16 newDomain;
	newDomain = bDomain + GetEndDomainIncrement ();
	e->SetDomain (newDomain);
    }
    else
    {
	Vector3int16 newDomain;
	newDomain = bDomain + GetEndDomainIncrement ();
	RuntimeAssert (newDomain == e->GetDomain (), "Inconsistent domains");
    }
}


G3D::Vector3int16 OrientedEdge::GetEndDomainIncrement () const
{
    if (m_reversed)
	return Vector3int16 (0,0,0) - m_edge->GetEndDomainIncrement ();
    else
	return m_edge->GetEndDomainIncrement ();;
}
