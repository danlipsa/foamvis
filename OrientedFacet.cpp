/**
 * @file OrientedFacet.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFacet class
 */

#include "OrientedFacet.h"

ostream& operator<< (ostream& ostr, OrientedFacet& of)
{
    ostr << (of.m_reversed ? "(R)" : "(N)");
    if (of.m_reversed)
	of.m_facet->ReversePrint (ostr);
    else
	ostr << *of.m_facet;
    return ostr;
}
