/**
 * @file Vertex.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Vertex class
 */
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Edge.h"
#include "ParsingDriver.h"
#include "EvolverData_yacc.h"
#include "DebugStream.h"
#include "Debug.h"
#include "Body.h"
#include "Data.h"
#include "Vertex.h"

// Private Functions
// ======================================================================
size_t hash_value (Vertex const& v)
{
    return Vertex::Hash () (v);
}

// Static Fields
// ======================================================================
AttributesInfo* Vertex::m_infos;

// Methods
// ======================================================================
Vertex::Vertex(float x, float y, float z, 
	       size_t originalIndex, Data* data, ElementStatus::Name status) :
    G3D::Vector3 (x, y, z),
    Element(originalIndex, data, status),
    m_adjacentPhysicalEdgesCount (0), m_physical (false)
{}

Vertex::Vertex (const G3D::Vector3* position, Data* data) : 
    G3D::Vector3 (position->x, position->y, position->z),
    Element (Element::INVALID_INDEX, data, ElementStatus::ORIGINAL),
    m_adjacentPhysicalEdgesCount (0), m_physical (false)
{}

Vertex::Vertex (const G3D::Vector3* position, Data* data,
		const G3D::Vector3int16& domainIncrement) : 
    G3D::Vector3 (position->x, position->y, position->z),
    Element (Element::INVALID_INDEX, data, ElementStatus::ORIGINAL),
    m_adjacentPhysicalEdgesCount (0), m_physical (false)
{
    TorusTranslate (domainIncrement);
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

void Vertex::TorusTranslate (const G3D::Vector3int16& domainIncrement)
{
    static_cast<G3D::Vector3>(*this) = 
	m_data->GetPeriods ().TorusTranslate (*this, domainIncrement);
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

Vertex* Vertex::CreateDuplicate (
    const G3D::Vector3int16& domainIncrement)
{
    SetStatus (ElementStatus::DUPLICATE_MADE);
    Vertex* duplicate = new Vertex (*this);
    duplicate->SetStatus (ElementStatus::DUPLICATE);
    duplicate->TorusTranslate (domainIncrement);
    return duplicate;
}

G3D::Vector3int16 Vertex::GetTorusLocation () const
{
    return m_data->GetPeriods ().GetTorusLocation (*this);
}


// Static and Friend functions
// ======================================================================
ostream& operator<< (ostream& ostr, const Vertex& v)
{
    ostr << "Vertex " << v.GetOriginalIndex () << " "
	 << v.GetStatus ()
	 << static_cast<const G3D::Vector3&>(v)
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr);
}

void Vertex::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS),
        new IntegerVectorAttributeCreator());
}
