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
    ostringstream id, ostr;
    id << (m_reversed ? "R" : "N") 
       << GetId () << " "
       << GetColoredElement ()->GetColor ();
    ostr << setw (15) << id.str () << ends;
    return ostr.str ();
}

size_t OrientedElement::GetId () const
{
    return GetColoredElement ()->GetId ();
}
