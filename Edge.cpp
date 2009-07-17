/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "Edge.h"

ostream& operator<< (ostream& ostr, Edge& e)
{
    if (&e == 0)
	ostr << "NULL";
    else
	ostr << "Edge: " << *e.m_begin << ", " << *e.m_end;
    return ostr;
}

void Edge::ReversePrint (ostream& ostr)
{
    if (this == 0)
	ostr << "NULL";
    else
	ostr << "Edge: " << *m_end << ", " << *m_begin;
}
