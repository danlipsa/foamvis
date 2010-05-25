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
const G3D::Vector3int16 Vertex::m_unitVector3int16[3] = {
    G3D::Vector3int16 (1, 0, 0),
    G3D::Vector3int16 (0, 1, 0),
    G3D::Vector3int16 (0, 0, 1)
};

// Methods
// ======================================================================
Vertex::Vertex(float x, float y, float z, 
	       size_t id, ElementStatus::Name status) :
    G3D::Vector3 (x, y, z),
    Element(id, status),
    m_adjacentPhysicalEdgesCount (0), m_physical (false)
{}

Vertex::Vertex (const G3D::Vector3& position) : 
    G3D::Vector3 (position),
    Element (Element::INVALID_INDEX, ElementStatus::ORIGINAL),
    m_adjacentPhysicalEdgesCount (0), m_physical (false)
{}


void Vertex::AddEdgePartOf (Edge* edge) 
{
    m_adjacentEdges.push_back (edge);
    if (edge->IsPhysical ())
	m_adjacentPhysicalEdgesCount++;
}

G3D::Vector3int16 Vertex::GetDomain () const
{
    return G3D::Vector3int16 (0, 0, 0);
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


// Static and Friend functions
// ======================================================================
ostream& operator<< (ostream& ostr, const Vertex& v)
{
    ostr << "Vertex " << v.GetId () << " "
	 << v.GetStatus ()
	 << static_cast<const G3D::Vector3&>(v)
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr);
}

void Vertex::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    auto_ptr<AttributeCreator> ac (new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);
    ac.reset (new IntegerVectorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS), ac);
}
