/**
 * @file Vertex.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Vertex class
 */
#include "Vertex.h"
#include "Edge.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "EvolverData_yacc.h"
#include "DebugStream.h"
#include "Debug.h"
#include "Body.h"
#include "Data.h"

ostream& operator<< (ostream& ostr, const Vertex& v)
{
    ostr << "Vertex " << v.GetOriginalIndex ()
	 << (v.IsDuplicate () ? " DUPLICATE": "")
	 << static_cast<const G3D::Vector3&>(v)
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr);
}

AttributesInfo* Vertex::m_infos;

Vertex::Vertex(float x, float y, float z, 
	       unsigned int originalIndex, Data* data, bool duplicate) :
    G3D::Vector3 (x, y, z),
    Element(originalIndex, data, duplicate),
    m_adjacentPhysicalEdgesCount (0)
{}

Vertex::Vertex (const G3D::Vector3* position, Data* data) : 
    G3D::Vector3 (position->x, position->y, position->z),
    Element (Element::INVALID_INDEX, data, false),
    m_adjacentPhysicalEdgesCount (0)
{}

Vertex::Vertex (const G3D::Vector3* position, Data* data,
		const G3D::Vector3int16& domainIncrement) : 
    G3D::Vector3 (position->x, position->y, position->z),
    Element (Element::INVALID_INDEX, data, false),
    m_adjacentPhysicalEdgesCount (0)
{
    AdjustPosition (domainIncrement);
}


void Vertex::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

void Vertex::AddAdjacentEdge (Edge* edge) 
{
    m_adjacentEdges.push_back (edge);
    if (edge->IsPhysical ())
	m_adjacentPhysicalEdgesCount++;
}

G3D::Vector3int16 Vertex::GetDomain () const
{
    return G3D::Vector3int16 (0, 0, 0);
}

void Vertex::AdjustPosition (const G3D::Vector3int16& domainIncrement)
{
    for (int i = 0; i < 3; i++)
    {
	*this += m_data->GetPeriod(i) * domainIncrement[i];
    }
}

bool Vertex::operator< (const Vertex& other) const
{
    return x < other.x ||
	(x == other.x && y < other.y) ||
	(x == other.x && y == other.y && z < other.z);
}

bool Vertex::operator== (const Vertex& other) const
{
    return static_cast<const G3D::Vector3&>(*this) ==
	static_cast<const G3D::Vector3&>(other);
}

size_t hash_value (Vertex const& v)
{
    return Vertex::Hash () (v);
}

Vertex* Vertex::CreateDuplicate (
    const G3D::Vector3int16& domainIncrement) const
{
    Vertex* duplicate = new Vertex (*this);
    duplicate->SetDuplicate (true);
    duplicate->AdjustPosition (domainIncrement);
    return duplicate;
}
