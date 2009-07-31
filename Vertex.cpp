/**
 * @file Point.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Point class
 */
#include "Vertex.h"
#include "lexYacc.h"
#include "AttributeInfo.h"
#include "foam_yacc.h"


ostream& operator<< (ostream& ostr, Vertex& p)
{
    if (&p == 0)
	return ostr << "NULL";
    else
	return ostr << "Point: " << p.m_x << ", " << p.m_y << ", " 
		    << p.m_z;
}

void Vertex::SetDefaultAttributes (AttributesInfo& info)
{
    info.AddAttributeInfo (
	KeywordString(ORIGINAL), new IntegerAttributeCreator());
}
