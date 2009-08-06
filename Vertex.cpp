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
	ostr << "NULL";
    else
	ostr << "Point: " << p.m_x << ", " << p.m_y << ", " 
	     << p.m_z << " Vertex attributes: ";
    return p.PrintAttributes (ostr, *Vertex::m_infos);
}

AttributesInfo* Vertex::m_infos;

void Vertex::SetDefaultAttributes (AttributesInfo& infos)
{
    m_infos = &infos;
    infos.AddAttributeInfo (
	KeywordString(ORIGINAL), new IntegerAttributeCreator());
}
