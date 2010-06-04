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
    Element(id, status)
{}

Vertex::Vertex (const G3D::Vector3& position) : 
    G3D::Vector3 (position),
    Element (Element::INVALID_INDEX, ElementStatus::ORIGINAL)
{}


void Vertex::AddEdgePartOf (Edge* edge) 
{
    m_edgesPartOf.push_back (edge);
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

bool Vertex::IsPhysical (size_t dimension, bool isQuadratic) const 
{
    if (dimension == 3)
	return count_if (m_edgesPartOf.begin (), m_edgesPartOf.end (),
			 boost::bind (&Edge::IsPhysical, _1, 
				      dimension, isQuadratic)) == 4;
    else
	if (isQuadratic)
	    return true;
	else
	    return m_edgesPartOf.size () >= 3;
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
    auto_ptr<AttributeCreator> ac (new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);
    ac.reset (new IntegerVectorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS), ac);
}
