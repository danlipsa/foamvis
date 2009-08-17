/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "Edge.h"
#include "lexYacc.h"
#include "AttributeInfo.h"
#include "foam_yacc.h"

ostream& operator<< (ostream& ostr, Edge& e)
{
    if (&e == 0)
        ostr << "NULL";
    else
        ostr << "Edge: " << *e.m_begin << ", " << *e.m_end << 
            " Edge attributes: ";
    return e.PrintAttributes (ostr, *Edge::m_infos);
}

AttributesInfo* Edge::m_infos;

void Edge::ReversePrint (ostream& ostr)
{
    if (this == 0)
        ostr << "NULL";
    else
        ostr << "Edge: " << *m_end << ", " << *m_begin;
}

void Edge::StoreDefaultAttributes (AttributesInfo& infos)
{
    m_infos = &infos;
    infos.AddAttributeInfo (
        KeywordString(ORIGINAL), new IntegerAttributeCreator());
}
