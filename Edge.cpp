/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Edge.h"
#include "EvolverData_yacc.h"
#include "Data.h"
#include "Debug.h"
#include "ParsingDriver.h"
#include "Vertex.h"

ostream& operator<< (ostream& ostr, const Edge& e)
{
    ostr << "Edge " << e.GetOriginalIndex () 
	 << (e.IsDuplicate () ? " DUPLICATE" : "")
	 << ": "
	 << static_cast<G3D::Vector3>(*e.m_begin) << ", " 
	 << static_cast<G3D::Vector3>(*e.m_end)
	 << " Adjacent faces(" << e.m_adjacentFaces.size () << ")"
	 << " Edge attributes: ";
    return e.PrintAttributes (ostr);
}

AttributesInfo* Edge::m_infos;


Edge::Edge (Vertex* begin, Vertex* end, G3D::Vector3int16& endDomainIncrement, 
	    size_t originalIndex, Data* data, bool duplicate):
    Element(originalIndex, data, duplicate),
    m_begin (begin), m_end (end), m_endDomainIncrement (endDomainIncrement), 
    m_physical (false)
{
    if (m_data->IsTorus () && ! m_duplicate)
    {
	if (m_endDomainIncrement == G3D::Vector3int16(0, 0, 0))
	    return;
	m_end = m_data->GetVertexDuplicate (*m_end, m_endDomainIncrement);
    }
}

Edge::Edge (Vertex* begin, size_t originalIndex) :
    Element (originalIndex, 0, false),
    m_begin (begin), m_end (0), m_physical (false)
{}

Edge::Edge (const Edge& o) : 
    Element (o.GetOriginalIndex (), o.GetData (), true),
    m_begin (o.GetBegin ()), m_end (o.GetEnd ()),
    m_endDomainIncrement (o.GetEndDomainIncrement ()), m_physical (false)
{}


void Edge::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
    const char* colorString = 
        ParsingDriver::GetKeywordString(parser::token::COLOR);
    // load the color attribute and nothing else
    infos->Load (colorString);
    infos->AddAttributeInfo (colorString, new ColorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS),
        new IntegerVectorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::DENSITY),
        new RealAttributeCreator());
}

short Edge::DomainIncrementCharToNumber (char sign)
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
	RuntimeAssert (false, "Invalid sign: ", sign);
	return 0;
    }
}

vector<Face*>& Edge::GetAdjacentFaces () 
{
    return m_adjacentFaces;
}

G3D::Vector3 Edge::GetBegin (const G3D::Vector3* end) const
{
    return *end + (*GetBegin () - *GetEnd ());
}

G3D::Vector3int16 Edge::IntToDomainIncrement (int value)
{
    G3D::Vector3int16 result;
    const int DIMENSIONS = 3;
    const int POSSIBILITIES = 3; // *, -, +
    for (int i = 0; i < DIMENSIONS; i++)
    {
	result[i] = (value % POSSIBILITIES) - 1; // we store -1, 0 or 1
	value /= POSSIBILITIES;
    }
    return result;
}

void Edge::UpdateVerticesAdjacency ()
{
    this->GetBegin ()->AddAdjacentEdge (this);
    this->GetEnd ()->AddAdjacentEdge (this);
}

bool Edge::operator< (const Edge& other) const
{
    return GetOriginalIndex () < other.GetOriginalIndex () ||
	(GetOriginalIndex () == other.GetOriginalIndex () &&
	 *GetBegin () < *other.GetBegin ());
}

bool Edge::operator== (const Edge& other) const
{
    return GetOriginalIndex () == other.GetOriginalIndex () &&
	*GetBegin () == *other.GetBegin ();
}

bool Edge::IsZero () const
{
    return (*GetEnd () - *GetBegin ()).isZero ();
}

Edge* Edge::CreateDuplicate (const G3D::Vector3& newBegin) const
{
    G3D::Vector3int16 domainIncrement = m_data->GetDomainIncrement (
	*GetBegin (), newBegin);
    Vertex* beginDuplicate = m_data->GetVertexDuplicate (
	*GetBegin (), domainIncrement);
    Vertex* endDuplicate = m_data->GetVertexDuplicate (
	*GetEnd (), domainIncrement);

    Edge* duplicate = new Edge (*this);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    return duplicate;
}
