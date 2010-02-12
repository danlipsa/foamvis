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
    calculateAdjacencyDomains (const Vertex* known) : m_known (known) {}
    void operator () (const Edge* e)
    {
	Vector3int16 knownDomain = m_known->GetDomain ();
	Vector3int16 newDomain;
	assert (knownDomain != Vertex::INVALID_DOMAIN);
	if (m_known == e->GetBegin ())
	{
	    if (e->GetEnd ()->GetDomain () != Vertex::INVALID_DOMAIN)
	    {
		// {
		//     newDomain = knownDomain + e->GetDomainIncrement ();
		//     if (e->GetEnd ()->GetDomain () != newDomain)
		//     {
		// 	cdbg << "WARNING: (" 
		// 	     << e->GetBegin ()->GetOriginalIndex () + 1
		// 	     << ", " << e->GetEnd ()->GetOriginalIndex () + 1 
		// 	     << ")"
		// 	     << " domains: " << newDomain << ", " 
		// 	     << e->GetEnd ()->GetDomain () << endl;
		//     }

		//     //assert (e->GetEnd ()->GetDomain () == newDomain);
		// }
		return;
	    }
	    newDomain = knownDomain + e->GetDomainIncrement ();
	    e->GetEnd ()->SetDomain (newDomain);
	    // cdbg << e->GetOriginalIndex () + 1<< ":"
	    // 	 << e->GetDomainIncrement () << ":"
	    // 	 << "(" << e->GetBegin ()->GetOriginalIndex () + 1
	    // 	 << ", " << e->GetEnd ()->GetOriginalIndex () + 1 << ")"
	    // 	 << " domain " << newDomain << endl;
	    Vertex::CalculateDomains (e->GetEnd ());
	}
	else
	{
	    assert (m_known == e->GetEnd ());
	    if (e->GetBegin ()->GetDomain () != Vertex::INVALID_DOMAIN)
	    {
		// {
		//     newDomain = knownDomain - e->GetDomainIncrement ();
		//     if (e->GetBegin ()->GetDomain () != newDomain)
		//     {
		// 	cdbg << "WARNING: (" 
		// 	     << e->GetBegin ()->GetOriginalIndex () + 1
		// 	     << ", " << e->GetEnd ()->GetOriginalIndex () + 1 
		// 	     << ")"
		// 	     << " domains: " << newDomain << ", " 
		// 	     << e->GetBegin ()->GetDomain () << endl;
		//     }
		//     //assert (e->GetBegin ()->GetDomain () == newDomain);
		// }
		return;
	    }
	    newDomain = knownDomain - e->GetDomainIncrement ();
	    e->GetBegin ()->SetDomain (newDomain);
	    // cdbg << e->GetOriginalIndex () + 1 << ":"
	    // 	 << e->GetDomainIncrement () << ":"
	    // 	 << "(" << e->GetBegin ()->GetOriginalIndex () + 1
	    // 	 << ", " << e->GetEnd ()->GetOriginalIndex () + 1 << ")"
	    // 	 << " domain " << newDomain << endl;
	    Vertex::CalculateDomains (e->GetBegin ());
	}
    }

private:
    const Vertex* m_known;
};


void Vertex::CalculateDomains (Vertex* known)
{
    for_each (known->m_adjacentEdges.begin (), 
	      known->m_adjacentEdges.end (),
	      calculateAdjacencyDomains (known));
}
