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

ostream& operator<< (ostream& ostr, Vertex& v)
{
    ostr << "Vertex " << v.GetOriginalIndex ()
	 << (v.IsDuplicate () ? " DUPLICATE": "")
	 << static_cast<Vector3&>(v)
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr, *Vertex::m_infos);
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

void Vertex::StoreDefaultAttributes (AttributesInfo& infos)
{
    using EvolverData::parser;
    m_infos = &infos;
    infos.AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

void Vertex::AddAdjacentEdge (Edge* edge) 
{
    m_adjacentEdges.push_back (edge);
    if (edge->IsPhysical ())
	m_adjacentPhysicalEdgesCount++;
}

G3D::Vector3int16 Vertex::GetDomain ()
{
    return Vector3int16 (0, 0, 0);
}

void Vertex::AdjustPosition (const G3D::Vector3int16& domainIncrement)
{
    for (int i = 0; i < 3; i++)
    {
	*this += m_data->GetPeriod(i) * domainIncrement[i];
    }
}

