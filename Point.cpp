/**
 * @file Point.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Point class
 */
#include "Point.h"

ostream& operator<< (ostream& ostr, Point& p)
{
    if (&p == 0)
	return ostr << "NULL";
    else
	return ostr << "Point: " << p.m_x << ", " << p.m_y << ", " 
		    << p.m_z;
}
