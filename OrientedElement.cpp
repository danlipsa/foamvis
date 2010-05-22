/**
 * @file OrientedElement.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedElement class
 */

#include "OrientedElement.h"
#include "Element.h"

string OrientedElement::GetSignedIdString () const
{
    ostringstream ostr;
    ostr << (m_reversed ? "R" : "N") << setw(3) << GetId () << ends;
    return ostr.str ();
}

size_t OrientedElement::GetId () const
{
    return GetElement ()->GetId ();
}
