/**
 * @file Point.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Point class
 */
#include "Vertex.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "EvolverData_yacc.h"


ostream& operator<< (ostream& ostr, const Point& p)
{
    if (&p == 0)
        ostr << "NULL";
    else
        ostr << "Point: " << p.GetX () << ", " << p.GetY () << ", " 
             << p.GetZ ();
    return ostr;
}
