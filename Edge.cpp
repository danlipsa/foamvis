#include "Edge.h"

using namespace std;

ostream& operator<< (ostream& ostr, Edge& e)
{
    ostr << "Edge: ";
    if (e.m_begin == 0)
	ostr << "NULL";
    else
	ostr << *e.m_begin;
    ostr << ", ";
    if (e.m_end == 0)
	ostr << "NULL";
    else
	ostr << *e.m_end;
    return ostr;
}
