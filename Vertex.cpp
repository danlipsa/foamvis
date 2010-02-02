/**
 * @file Vertex.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Vertex class
 */
#include "Vertex.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "EvolverData_yacc.h"


ostream& operator<< (ostream& ostr, const Vertex& v)
{
    using namespace G3D;
    if (&v == 0)
        ostr << "NULL";
    else
        ostr << static_cast<const Vector3&>(v)
	     << " EDGES " << v.m_edges.size ()
	     << " Vertex attributes: ";
    return v.PrintAttributes (ostr, *Vertex::m_infos);
}

AttributesInfo* Vertex::m_infos;

void Vertex::StoreDefaultAttributes (AttributesInfo& infos)
{
    using EvolverData::parser;
    m_infos = &infos;
    infos.AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

