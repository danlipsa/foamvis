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

ostream& operator<< (ostream& ostr, const Vertex& v)
{
    ostr << "Vertex " << v.GetOriginalIndex () << " "
	 << v.GetStatus ()
	 << static_cast<const G3D::Vector3&>(v)
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr);
}

AttributesInfo* Vertex::m_infos;

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
    Translate (domainIncrement);
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

void Vertex::Translate (const G3D::Vector3int16& domainIncrement)
{
    static_cast<G3D::Vector3>(*this) = 
	m_data->GetPeriods ().Translate (*this, domainIncrement);
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
    const G3D::Vector3int16& domainIncrement)
{
    SetStatus (ElementStatus::DUPLICATE_MADE);
    Vertex* duplicate = new Vertex (*this);
    duplicate->SetStatus (ElementStatus::DUPLICATE);
    duplicate->Translate (domainIncrement);
    return duplicate;
}

G3D::Vector3int16 Vertex::GetDomainIncrement () const
{
    using G3D::Vector3int16;
    using G3D::Vector3;
    using G3D::Plane;
    typedef pair<Vector3int16, Vector3int16> PlanePair;
    boost::array<PlanePair, 3> planePairs = {{
	    PlanePair (Vector3int16 (2, 1, 0), Vector3int16 (1, 0, 0)),// X
	    PlanePair (Vector3int16 (0, 2, 1), Vector3int16 (0, 1, 0)),// Y
	    PlanePair (Vector3int16 (1, 0, 2), Vector3int16 (0, 0, 1)) // Z
	}};
    Vector3 one, two, three, translate;
    Plane p;
    Vector3int16 domainIncrement;
    BOOST_FOREACH (PlanePair planePair, planePairs)
    {
	Vector3int16 plane = planePair.first;
	Vector3int16 increment = planePair.second;
	one = m_data->GetPeriod (plane.x);
	two = Vector3::zero ();
	three = m_data->GetPeriod (plane.y);
	translate = m_data->GetPeriod (plane.z);
	p = Plane (one, two, three);
	if (! p.halfSpaceContainsFinite (*this))
	    domainIncrement -= increment;;
	p = Plane(three + translate, two + translate, one + translate);
	if (! p.halfSpaceContainsFinite (*this))
	    domainIncrement += increment;;
    }
    return domainIncrement;
}
