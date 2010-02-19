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
	 << " Vertex attributes: ";
    return v.PrintAttributes (ostr, *Vertex::m_infos);
}

AttributesInfo* Vertex::m_infos;


Vertex::Vertex(unsigned int originalIndex, float x, float y, float z, 
	       bool duplicate) :
    G3D::Vector3 (x, y, z), Element(originalIndex, duplicate),
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

void Vertex::AddAdjacentEdge (const Edge* edge) 
{
    m_adjacentEdges.push_back (edge);
    if (edge->IsPhysical ())
	m_adjacentPhysicalEdgesCount++;
}

