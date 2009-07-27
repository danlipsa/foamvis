/**
 * @file   OrientedFacet.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFacet class
 */
#include "OrientedFacet.h"

using namespace std;

ostream& operator<< (ostream& ostr, OrientedEdge& oe)
{
    ostr << (oe.m_reversed ? "(R)" : "(N)");
    if (oe.m_reversed)
	oe.m_edge->ReversePrint (ostr);
    else
	ostr << *oe.m_edge;
    return ostr;
}
