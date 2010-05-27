/**
 * @file OrientedElement.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedElement class
 */

#include "OrientedElement.h"
#include "ColoredElement.h"

string OrientedElement::GetStringId () const
{
    ostringstream ostr;
    ostr << (m_reversed ? "R" : "N") 
	 << GetColoredElement ()->GetStringId ();
    return ostr.str ();
}

size_t OrientedElement::GetId () const
{
    return GetColoredElement ()->GetId ();
}
