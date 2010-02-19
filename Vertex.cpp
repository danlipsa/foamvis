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

ostream& operator<< (ostream& ostr, const Vertex& v)
{
    using namespace G3D;
    ostr << static_cast<const Vector3&>(v)
	 << " Periodic domain: " << v.m_domain
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr, *Vertex::m_infos);
}

AttributesInfo* Vertex::m_infos;


Vertex::Vertex(unsigned int originalIndex, float x, float y, float z) :
    G3D::Vector3 (x, y, z), Element(originalIndex),
    m_adjacentPhysicalEdgesCount (0), 
    m_domain(INVALID_DOMAIN)
{}

const G3D::Vector3int16 Vertex::INVALID_DOMAIN(
    numeric_limits<short>::max (),
    numeric_limits<short>::max (),
    numeric_limits<short>::max ());

void Vertex::StoreDefaultAttributes (AttributesInfo& infos)
{
    using EvolverData::parser;
    m_infos = &infos;
    infos.AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

void Vertex::AddAdjacentEdge (const Edge* edge) 
{
    m_adjacentEdges.push_back (edge);
    if (edge->IsPhysical ())
	m_adjacentPhysicalEdgesCount++;
}

class calculateAdjacencyDomains
{
public:
    calculateAdjacencyDomains (const Vertex* known, const Body* body) : 
	m_known (known), m_body (body) {}
    void operator () (const Edge* e)
    {
	Vector3int16 knownDomain = m_known->GetDomain ();
	Vector3int16 newDomain;
	RuntimeAssert (knownDomain != Vertex::INVALID_DOMAIN, 
	    "The known domain is invalid");
	if (m_known == e->GetBegin ())
	{
	    if (! m_body->HasVertex (e->GetEnd ()))
		return;
	    if (e->GetEnd ()->GetDomain () != Vertex::INVALID_DOMAIN)
	    {
		newDomain = knownDomain + e->GetEndDomainIncrement ();
		RuntimeAssert (e->GetEnd ()->GetDomain () == newDomain,
			       "Inconsistent intersections");
		return;
	    }
	    newDomain = knownDomain + e->GetEndDomainIncrement ();
	    e->GetEnd ()->SetDomain (newDomain);
	    Vertex::CalculateDomains (e->GetEnd (), m_body);
	}
	else
	{
	    if (! m_body->HasVertex (e->GetBegin ()))
		return;
	    RuntimeAssert (m_known == e->GetEnd (),
			   "The known domain is not at the end of the edge");
	    if (e->GetBegin ()->GetDomain () != Vertex::INVALID_DOMAIN)
	    {

		newDomain = knownDomain - e->GetEndDomainIncrement ();
		RuntimeAssert (e->GetBegin ()->GetDomain () == newDomain,
			       "Inconsistent intersections");
		return;
	    }
	    newDomain = knownDomain - e->GetEndDomainIncrement ();
	    e->GetBegin ()->SetDomain (newDomain);
	    Vertex::CalculateDomains (e->GetBegin (), m_body);
	}
    }

private:
    const Vertex* m_known;
    const Body* m_body;
};


void Vertex::CalculateDomains (const Vertex* start, const Body* body)
{
    for_each (start->m_adjacentEdges.begin (), 
	      start->m_adjacentEdges.end (),
	      calculateAdjacencyDomains (start, body));
}

void Vertex::TranslateVertex (const G3D::Vector3* periods)
{
    (*this) += periods[0] * GetDomain ()[0] + periods[1] * GetDomain ()[1] +
	periods[2] * GetDomain ()[2];
}


