/**
 * @file   T1.cpp
 * @author Dan R. Lipsa
 * @date 7 May 2013
 *
 * Implementations for functionality declared in T1.h
 */

#include "T1.h"

// Methods T1Type
// ======================================================================
string T1::ToString () const
{
    ostringstream ostr;
    ostr << "type: " << m_type << ", position: " << m_position;
    return ostr.str ();
}

ostream& operator<< (ostream& ostr, const T1& t1)
{
    return ostr << t1.ToString ();
}
