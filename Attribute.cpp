/**
 * @file Attribute.cpp
 * @author Dan R. Lipsa
 *
 * Contains definition  of class members for all types  of attributes which
 * can be attached to vertices, edges, faces and bodies.
 */
#include "Attribute.h"

ostream& IntegerArrayAttribute::Print (ostream& ostr) const 
{
    ostream_iterator<int> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    return ostr;
}

ostream& RealArrayAttribute::Print (ostream& ostr) const
{
    ostream_iterator<float> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    return ostr;
}

ostream& operator<< (ostream& ostr,  Attribute& attribute)
{
    if (&attribute == 0)
        return ostr << "null";
    else
        return attribute.Print(ostr);
}
