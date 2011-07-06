/**
 * @file OrientedElement.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedElement class
 */

#include "OrientedElement.h"
#include "Element.h"

string OrientedElement::GetStringId () const
{
    ostringstream ostr;
    ostr << (m_reversed ? "R" : "N") 
	 << GetElement ()->GetStringId ();
    return ostr.str ();
}

string OrientedElement::GetOrientationString () const
{
    ostringstream ostr;
    ostr << (m_reversed ? "(R)" : "(N)");
    return ostr.str ();
}

size_t OrientedElement::GetId () const
{
    return GetElement ()->GetId ();
}
