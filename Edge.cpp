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
#include "Data.h"


ostream& operator<< (ostream& ostr,  Edge& e)
{
    ostr << "Edge " << e.GetOriginalIndex () 
	 << (e.IsDuplicate () ? " DUPLICATE" : "")
	 << ": "
	 << static_cast<G3D::Vector3>(*e.m_begin) << ", " 
	 << static_cast<G3D::Vector3>(*e.m_end)
	 << " Adjacent faces(" << e.m_adjacentFaces.size () << ")"
	 << " Edge attributes: ";
    return e.PrintAttributes (ostr, *Edge::m_infos);
}

AttributesInfo* Edge::m_infos;


Edge::Edge (Vertex* begin, Vertex* end, G3D::Vector3int16& endDomainIncrement, 
	    unsigned int originalIndex, Data* data, bool duplicate):
    Element(originalIndex, data, duplicate),
    m_begin (begin), m_end (end), m_endDomainIncrement (endDomainIncrement)
{
    if (m_data->IsTorus () && ! m_duplicate)
    {
	if (m_endDomainIncrement == G3D::Vector3int16(0, 0, 0))
	    return;
	m_end = m_data->GetVertexDuplicate (*m_end, m_endDomainIncrement);
    }
}

Edge::Edge (Vertex* begin, unsigned int originalIndex) :
    Element (originalIndex, 0, false),
    m_begin (begin), m_end (0)
{}


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

vector<Face*>& Edge::GetAdjacentFaces () 
{
    return m_adjacentFaces;
}

G3D::Vector3 Edge::GetBegin (G3D::Vector3* end)
{
    return *end + (*GetBegin () - *GetEnd ());
}
