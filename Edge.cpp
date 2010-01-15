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
using namespace std;

ostream& operator<< (ostream& ostr, Edge& e)
{
    if (&e == 0)
        ostr << "NULL";
    else
        ostr << "Edge: " << *e.m_begin << ", " << *e.m_end << 
            " Edge attributes: ";
    return e.PrintAttributes (ostr, *Edge::m_infos);
}

void Edge::ReversePrint (ostream& ostr)
{
    if (this == 0)
        ostr << "NULL";
    else
        ostr << "Edge: " << *m_end << ", " << *m_begin << 
            " Edge attributes: ";
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

void Edge::CalculateAverage ()
{
    if (! m_averageCalculated)
    {
	m_averageCalculated = true;
	m_average.SetX ((m_begin->GetX () + m_end->GetX ()) / 2);
	m_average.SetY ((m_begin->GetY () + m_end->GetY ()) / 2);
	m_average.SetZ ((m_begin->GetZ () + m_end->GetZ ()) / 2);
    }
}
