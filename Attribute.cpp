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
    ostr << "[";
    ostream_iterator<int> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    ostr << "]";
    return ostr;
}

ostream& RealArrayAttribute::Print (ostream& ostr) const
{
    ostr << "[";
    ostream_iterator<double> o (ostr, " ");
    copy (m_values->begin (), m_values->end (), o);
    ostr << "]";
    return ostr;
}

ostream& operator<< (ostream& ostr, const Attribute& attribute)
{
    return attribute.Print(ostr);
}

ostream& operator<< (ostream& ostr, const Attribute* attribute)
{
    return ostr << *attribute;
}
