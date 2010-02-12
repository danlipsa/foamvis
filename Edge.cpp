/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "Edge.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "EvolverData_yacc.h"


ostream& operator<< (ostream& ostr, const Edge& e)
{
    if (&e == 0)
        ostr << "NULL";
    else
        ostr << "Edge: " << *e.m_begin << ", " << *e.m_end 
	     << " ADJACENT FACES " << e.m_adjacentFaces.size ()
	     << " Edge attributes: ";
    return e.PrintAttributes (ostr, *Edge::m_infos);
}

AttributesInfo* Edge::m_infos;

void Edge::ReversePrint (ostream& ostr)
{
    ostr << "Edge: " << *m_end << ", " << *m_begin
	 << " ADJACENT FACES " << m_adjacentFaces.size ()
	 << " Edge attributes: ";
    PrintAttributes (ostr, *Edge::m_infos);
}

void Edge::StoreDefaultAttributes (AttributesInfo& infos)
{
    using EvolverData::parser;
    m_infos = &infos;
    infos.AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

short Edge::SignToNumber (char sign)
{
    switch (sign)
    {
    case '*':
	return 0;
    case '+':
	return 1;
    case '-':
	return -1;
    default:
	throw invalid_argument (string ("Invalid sign: ") + sign);
    }
}
