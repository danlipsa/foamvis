/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"

ostream& operator<< (ostream& ostr, OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R)" : "(N)");
    if (of.m_reversed)
	of.m_face->ReversePrint (ostr);
    else
	ostr << *of.m_face;
    return ostr;
}
